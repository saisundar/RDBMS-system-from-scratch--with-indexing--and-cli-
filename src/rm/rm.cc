#include "rm.h"

RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
	if(!_rm)
		_rm = new RelationManager();

	return _rm;
}

RelationManager::RelationManager()
{
	rbfm = RecordBasedFileManager::instance();
	systemCatalog = "System_Catalog";
	tableCatalogConcat = new char[5];
	*(tableCatalogConcat) = 'c';
	*(tableCatalogConcat+1) = 'a';
	*(tableCatalogConcat+2) =  't';
	*(tableCatalogConcat+3) = '_';
	*(tableCatalogConcat+4) = 0;

	if(FileExists(systemCatalog))
		rbfm->openFile(systemCatalog.c_str(),systemHandle);

	Attribute attr;
	attr.name = "tableName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	systemDescriptor.push_back(attr);

	attr.name = "tableType";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)10;
	systemDescriptor.push_back(attr);

	attr.name = "numCols";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	systemDescriptor.push_back(attr);

	attr.name = "tableName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	tableDescriptor.push_back(attr);

	attr.name = "columnName";
	attr.type = TypeVarChar;
	attr.length = (AttrLength)30;
	tableDescriptor.push_back(attr);

	attr.name = "columnType";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	tableDescriptor.push_back(attr);

	attr.name = "columnPosition";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	tableDescriptor.push_back(attr);

	attr.name = "maxSize";
	attr.type = TypeInt;
	attr.length = (AttrLength)4;
	tableDescriptor.push_back(attr);
}

RelationManager::~RelationManager()
{

	rbfm->closeFile(systemHandle);
	// ********* ???
	free(rbfm);
	free(tableCatalogConcat);
}

RC RelationManager::insertEntryForTableCatalog(FileHandle &tableCatalogHandle, const string &tableName, const string &columnName, INT32 columnType, INT32 columnPosition, INT32 maxSize)
{
	dbgn2("<----------------------------In Insert Entry For Table Catalog------------------------->","");
	INT32 l1 = strlen(tableName.c_str());
	INT32 l2 = strlen(columnName.c_str());
	INT32 dataLength = 20 + l1 + l2;
	void* data = malloc(dataLength);
	BYTE* tableData = (BYTE*)data;
	//Copy length of tableName
	INT32 temp = l1;
	memcpy(tableData, &temp, 4);
	tableData += 4;

	//copy tableName
	memcpy(tableData,tableName.c_str(),l1);
	tableData+=l1;

	//copy length of columnName
	temp = l2;
	memcpy(tableData, &temp, 4);
	tableData+=4;

	//copy columnName
	memcpy(tableData,columnName.c_str(),l2);
	tableData+=l2;

	//copy columType
	temp = columnType;
	memcpy(tableData, &temp, 4);
	tableData+=4;

	//copy columnPosition
	temp = columnPosition;
	memcpy(tableData, &temp, 4);
	tableData+=4;

	//copy maxSize
	temp = maxSize;
	memcpy(tableData, &temp, 4);
	tableData+=4;

	// Insert Record;
	RID systemRid;
	dbgn2("Record Being Inserted in Table Catalog: ", rbfm->printRecord(tableDescriptor, data));

	rbfm->insertRecord(tableCatalogHandle,tableDescriptor,data,systemRid);

	free(tableData);
	return 0;
}

RC RelationManager::insertEntryForSystemCatalog(const string &tableName, const string &tableType, INT32 numCols){
	dbgn2("<----------------------------In Insert Entry For System Catalog------------------------->","");
	INT32 l1 = strlen(tableName.c_str());
	dbgn2("l1",l1);
	INT32 l2 = strlen(tableType.c_str());
	dbgn2("l2",l2);
	INT32 dataLength = 12 + l1 + l2;
	void* tempData = malloc(dataLength);
	BYTE* systemData = (BYTE*)tempData;
	INT32 temp = l1;
	memcpy(systemData, &temp, 4);

	systemData += 4;
	memcpy(systemData,tableName.c_str(),l1);

	systemData+=l1;
	temp = l2;
	memcpy(systemData, &temp, 4);

	systemData+=4;
	memcpy(systemData,tableType.c_str(),l2);

	systemData+=l2;
	temp = 3;
	memcpy(systemData, &temp, 4);

	// Insert Record;
	RID systemRid;
	dbgn2("Record Being Inserted in System Catalog: ", rbfm->printRecord(systemDescriptor, tempData));
	rbfm->insertRecord(systemHandle,systemDescriptor,tempData,systemRid);

	free(tempData);
	return 0;
}

/*RC RelationManager::createRecordDescriptor(FileHandle &fileHandle, const string &tableName, const vector<Attribute> &recordDescriptor){
	dbgn2("<----------------------------In Create Record Descriptor------------------------->","");
	if(descriptors.find(tableName)!=descriptors.end()){
		dbgn2("Descriptor is already Created",", returning stored value");
		recordDescriptor = (FileHandle)descriptors[tableName];
		return 0;
	}
	dbgn2("Descriptor is being Created","");
	// ELse create Record Descriptor for that table
// ********************************************* Left to Implement ******************************
	// Insert Record Descriptor in Map
	descriptors.insert(std::pair< string ,vector<Attribute>>(tableName, recordDescriptor));
	return 0;
}*/

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
	dbgn2("<----------------------------In Create Table------------------------->","");
	if(!FileExists(systemCatalog)){
		// Create System Catalog if not created
		dbgn2("System catalog does not exist",", Creating IT !");
		rbfm->createFile(systemCatalog.c_str());
		rbfm->openFile(systemCatalog.c_str(),systemHandle);
		insertEntryForSystemCatalog(systemCatalog, "System", 3);

	}

	// Insert Record for new table in System_Catalog;
	insertEntryForSystemCatalog(tableName.c_str(), "User", attrs.size());

	// Create new file for the table
	if(rbfm->createFile(tableName.c_str())==-1){
		dbgn2("Create File Failed","(in createTable)");
		return -1;
	}

	// Create new file for the table catalog and associate fileHandle
	char * tableCatalogName =(char *)malloc(strlen(tableName.c_str())+5);
	tableCatalogName=strcpy(tableCatalogName,tableCatalogConcat);
	tableCatalogName=strcat(tableCatalogName,tableName.c_str());
	dbgn2("table Catalog Name: ",tableCatalogName);
	dbgn2("table Catalog concat: ",tableCatalogConcat);

	if(rbfm->createFile(tableCatalogName)==-1){
		dbgn2("Create Table Catalog Failed","");
		return -1;
	}
	FileHandle tableCatalogHandle;
	if(rbfm->openFile(tableCatalogName,tableCatalogHandle)==-1){
		dbgn2("Open Table Catalog Failed","");
		return -1;
	}

	// Insert Record for new Table Catalog in System_Catalog
	insertEntryForSystemCatalog(tableCatalogName, "System", 5);

	// Insert Records in Table Catalog
	for(int i=0;i<attrs.size();i++){
		Attribute tableAttr = attrs[i];
		insertEntryForTableCatalog(tableCatalogHandle, tableName, tableAttr.name, tableAttr.type, i, tableAttr.length);
	}
	if(rbfm->closeFile(tableCatalogHandle)==-1){
		dbgn2("Close Table Catalog Failed","");
		return -1;
	}
	return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
	dbgn2("<----------------------------In Destroy Table------------------------->","");
	RBFM_ScanIterator rbfmsi;

	// Make Application Layer Entry for tableName to insert in "ConditionAttribute" field in scan function for searching it in SystemCatalog
	INT32 length = strlen(tableName.c_str());
	void * tempData = malloc(4+length);
	BYTE* data = (BYTE*)tempData;
	dbgn2("length of table search string: ", length+4);
	memcpy(data,&length,4);
	data = data + 4;
	memcpy(data,tableName.c_str(),4);
	string conditionAttr = "tableName";
	vector<string> dummy;
	RID deleteRid;

	dbgn2("Searching For: ",tableName);
	if(rbfm->scan(systemHandle, systemDescriptor, conditionAttr, EQ_OP, (void*)data, dummy, rbfmsi)==-1)return -1;
	free(tempData);
	if(rbfmsi.getNextRecord(deleteRid, data)==RBFM_EOF){
		dbgn2("Record not found by scan iterator: ",tableName);
		return -1;
	}

	vector<Attribute> dummy2;
	if(rbfm->deleteRecord(systemHandle, dummy2, deleteRid)==-1){
		dbgn2("Could not delete","record for table in system catalog");
		return -1;
	}
	rbfmsi.close();

	// Make Application Layer Entry for tableName's Catalog to insert in "ConditionAttribute" field in scan function for searching it in SystemCatalog
	char * tableCatalogName =(char *)malloc(strlen(tableName.c_str())+5);
	tableCatalogName=strcpy(tableCatalogName,tableCatalogConcat);
	tableCatalogName=strcat(tableCatalogName,tableName.c_str());

	dbgn2("table Catalog Name: ",tableCatalogName);
	length = strlen(tableCatalogName);
	dbgn2("length of table catalog search string: ", length+4);
	tempData = malloc(4+length);
	data = (BYTE*)tempData;
	memcpy(data,&length,4);
	data = data + 4;
	memcpy(data,tableCatalogName,4);
	dbgn2("Searching For: ",tableCatalogName);
	rbfm->scan(systemHandle, systemDescriptor, conditionAttr, EQ_OP, (void*)data, dummy, rbfmsi);
	free(tempData);
	if(rbfmsi.getNextRecord(deleteRid, data)==RBFM_EOF){
		dbgn2("Record not found by scan iterator: ",tableName);
		return -1;
	}

	if(rbfm->deleteRecord(systemHandle, dummy2, deleteRid)==-1){
		dbgn2("Could not delete ","record for table CATALOG in system catalog");
		return -1;
	}
	rbfmsi.close();

	if(rbfm->destroyFile(tableName)==-1){
		dbgn2("Could not delete","file for the table");
		return -1;
	}

	return 0;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
	dbgn2("<----------------------------getAttributes------------------------->","");
	if(descriptors.find(tableName)!=descriptors.end()){
		dbgn2("Descriptor is already Created",", returning stored value");
		attrs = (vector<Attribute>)descriptors[tableName];
		return 0;
	}

	dbgn2("Descriptor is being Created","");
	// ELse create Record Descriptor for that table
	// ********************************************* Left to Implement ******************************
	// Insert Record Descriptor in Map
	char * tableCatalogName =(char *)malloc(strlen(tableName.c_str())+5);
	tableCatalogName=strcpy(tableCatalogName,tableCatalogConcat);
	tableCatalogName=strcat(tableCatalogName,tableName.c_str());
	FileHandle tableCatalogHandle;
	if(rbfm->openFile(tableCatalogName, tableCatalogHandle)==-1)
	{
		dbgn2("Failed to open",tableCatalogName);
		return -1;
	}

	// Create attributes necessary for scan iterator
	string conditionAttr ="columnPosition";
	INT32 columnPosition = 0,i=0;
	vector<string> projectedAttributes;
	RBFM_ScanIterator rbfmsi;
	RID dummyRid;
//	Attribute attr;
	void* data = malloc(42);

	// Create Projected attributes to retrieve relevant attributes for catalog table
	projectedAttributes.push_back("columnName");
	projectedAttributes.push_back("columnType");
	projectedAttributes.push_back("maxSize");

	// Scan through the table catalog file to create the record descriptor
	while(true){
		RBFM_ScanIterator rbfmsi;
		rbfm->scan(tableCatalogHandle, tableDescriptor, conditionAttr, EQ_OP, (void*)&columnPosition, projectedAttributes, rbfmsi);
		if(rbfmsi.getNextRecord(dummyRid, data)==RBFM_EOF)break; // If no record found break, since we have scanned through all the records in the file

		// Make Attribute object
		Attribute attr;
		BYTE* iterData = (BYTE*)data;
		INT32 nameLength = *((INT32*)iterData);
		iterData += 4;
		attr.name.resize(nameLength,0);
		dbgn1("attr namebefore copy",attr.name);
		//memcpy(attr.name,iterData,nameLength);

		for(i=0;i<nameLength;i++,iterData++)attr.name[i]=*(char *)iterData;

		attr.type = *((AttrType*)iterData);
		iterData  +=sizeof(INT32);
		attr.length = *((AttrLength*)iterData);
		dbgn1("attr name",attr.name);
		dbgn1("attr length",attr.length);
		dbgn1("attr type",attr.type);
		//Add attribute to record descriptor vector
		attrs.push_back(attr);
		columnPosition++;
		rbfmsi.close();
	}
	dbgn2("Length of recordDescriptor: ",columnPosition);


	descriptors.insert(std::pair< string ,vector<Attribute> >(tableName, attrs));
	return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
	dbgn2("<----------------------------In Insert Tuple (RM)------------------------->","");
	vector<Attribute> recordDescriptor;
	if(getAttributes(tableName, recordDescriptor)==-1){
		dbgn2("could not create Record descriptor","");
		return -1;
	}
	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1){
		dbgn2("could not create Record descriptor","");
		return -1;
	}
	if(rbfm->insertRecord(tableHandle,recordDescriptor,data,rid)==-1){
		dbgn2("could not insert the new record","");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1){
		dbgn2("could close the file","");
		return -1;
	}
	return 0;
}

RC RelationManager::deleteTuples(const string &tableName)
{
	dbgn2("<----------------------------In Delete Tuple's' (RM)------------------------->","");
	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1)return -1;
	if(rbfm->deleteRecords(tableHandle)==-1)return -1;
	if(rbfm->closeFile(tableHandle)==-1)return -1;
	return 0;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
	dbgn2("<----------------------------In Delete Tuple (RM)------------------------->","");
	FileHandle tableHandle;
	vector<Attribute> dummy;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1)
	{
		dbgn2("open file failed","ooops");
		return -1;
	}

	if(rbfm->deleteRecord(tableHandle,dummy,rid)==-1)
	{
		dbgn2("delete record file failed","ooops");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1)return -1;
	return 0;
	return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
	dbgn2("<----------------------------In Insert Tuple (RM)------------------------->","");
	vector<Attribute> recordDescriptor;
	if(getAttributes(tableName, recordDescriptor)==-1){
		dbgn2("could not create Record descriptor","");
		return -1;
	}

	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1){
		dbgn2("could not create Record descriptor","");
		return -1;
	}
	if(rbfm->updateRecord(tableHandle,recordDescriptor,data,rid)==-1){
		dbgn2("could not insert the new record","");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1){
		dbgn2("could close the file","");
		return -1;
	}
	return 0;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
	dbgn2("<----------------------------In Read Tuple (RM)------------------------->","");
	vector<Attribute> recordDescriptor;
	if(getAttributes(tableName, recordDescriptor)==-1){
		dbgn2("could not create Record descriptor","In Read Tuple (RM)");
		return -1;
	}

	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1){
		dbgn2("could not create Record descriptor","In Read Tuple (RM)");
		return -1;
	}
	if(rbfm->readRecord(tableHandle, recordDescriptor, rid, data)==-1){
		dbgn2("could not read the new record","In Read Tuple (RM)");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1){
		dbgn2("could close the file","In Read Tuple (RM)");
		return -1;
	}
	return 0;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
	dbgn2("<----------------------------In Read Attribute (RM)------------------------->","");
	vector<Attribute> recordDescriptor;
	if(getAttributes(tableName, recordDescriptor)==-1){
		dbgn2("could not create Record descriptor","In Read Attribute (RM))");
		return -1;
	}

	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1){
		dbgn2("could not create Record descriptor","IIn Read Attribute (RM)");
		return -1;
	}
	if(rbfm->readAttribute(tableHandle, recordDescriptor, rid, attributeName, data)==-1){
		dbgn2("could not read the attribute","In Read Attribute (RM)");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1){
		dbgn2("could close the file","In Read Attribute (RM)");
		return -1;
	}
	return 0;
}

RC RelationManager::reorganizePage(const string &tableName, const unsigned pageNumber)
{
	vector<Attribute> dummyDescriptor;
	FileHandle tableHandle;
	if(rbfm->openFile(tableName.c_str(),tableHandle)==-1){
		dbgn2("could not create Record descriptor","IIn Read Attribute (RM)");
		return -1;
	}
	if(rbfm->reorganizePage(tableHandle, dummyDescriptor, pageNumber)==-1){
		dbgn2("could not insert the new record","In Read Attribute (RM)");
		return -1;
	}
	if(rbfm->closeFile(tableHandle)==-1){
		dbgn2("could close the file","In Read Attribute (RM)");
		return -1;
	}
	return 0;
}

RC RelationManager::scan(const string &tableName,
		const string &conditionAttribute,
		const CompOp compOp,
		const void *value,
		const vector<string> &attributeNames,
		RM_ScanIterator &rm_ScanIterator)
{
	dbgn2("<----------------------------In scan (RM)------------------------->","");
return 0;
}

// Extra credit
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
	return -1;
}

// Extra credit
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
	return -1;
}

// Extra credit
RC RelationManager::reorganizeTable(const string &tableName)
{
	return -1;
}
