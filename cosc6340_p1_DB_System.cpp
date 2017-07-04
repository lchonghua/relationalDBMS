#include <regex>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <fstream>
#include <map>

using namespace std;


//*************************************************************
//                     Variables
//*************************************************************
struct tableinfo //to store an entry in the data dictionary
{
	string tablename;
	string fieldname;
	string fieldtype;
	string address;
};
vector<tableinfo> dictionary;//store the data dictionary
vector<tableinfo> extractedTableInfo, extractedTableInfo2;//hold the results from the function extractTableInfoFromDic(string tbName)
									//which extract the info of a specific table from data dictionary (so it will be easier to handle)
vector<string> command;//hold the key words from the command parsed by splitString functions
ifstream infile, infile2;
ofstream outfile;

//********************************e******************************
//					Function Prototyp
//***************************************************************

void splitString(string str);//parse the input command and store the result in vector<string> command(use white space as dilimiter)
void splitString_2(string str);//parse input command containing "( )", "," and double quotes
bool checkTable(string tbName);//check if table is already exist
vector<tableinfo> extractTableInfoFromDic(string tbName);//extract information of a specific table from dictionary.
bool checkField(vector<string>& command);//to check if the input values in the INSERT command match the types and number of the fields
bool findItemInVectorOfStruct(vector<tableinfo>& searchPool, string item);//a function to search for an item from a vector of struct,
																		//such as find a field name in the vector of extracted table information
map<string, string> getTableEntry(string str, vector<tableinfo>& tbInfo); //a function to store a tuple from a table into a map, use field name as key


//********************************e******************************
//					Main Function
//***************************************************************
int main()
{
	string str1,str2, inputCmd;
	tableinfo tempTable;
	bool repeat = true;
	bool found;
	
	//Open the data dictionary txt file. If exist, load the information into a data structure call "dictionary"
	//If not exist, create one.
	infile.open("DataDictionary.txt");
	if(infile)
	{
		while(!infile.eof())
		{
			istringstream ss;
			getline(infile, str1);
			ss.str(str1);
			ss >> tempTable.tablename >> tempTable.fieldname >> tempTable.fieldtype >> tempTable.address;  

			dictionary.push_back(tempTable);
		}
		infile.close();
	}
	while (repeat)
	{
		cout<<"\nPlease enter your command:"<<endl;
		getline(cin,inputCmd); 
		if(inputCmd == "EXIT")
			break;

		//*****************************************************************************
		//           Regular expressions to define the command syntax:
		//*****************************************************************************
		regex e1("CREATE\\s+[a-z0-9]+\\s*[(][a-z0-9]+\\s+(INT|STRING)(,\\s*[a-z0-9]+\\s+(INT|STRING))*[)]");
			//CREATE table (field1 INT|STRING, field2 INT|STRING, etc)
		regex e2("INSERT\\s+INTO\\s+[a-z0-9]+\\s*[(][a-zA-Z0-9\"\\s]+(,\\s*[a-zA-Z0-9\"\\s]+)*[)]");
			//INSERT INTO table(value1, value2, etc)
		regex e3("SELECT\\s+\\*\\s+FROM\\s+[a-z0-9]+,\\s*[a-z0-9]+\\s+WHERE\\s+[a-z0-9]+\\s*=\\s*[a-z0-9]+");
			//SELECT * FROM table1, table2 WHERE field1=field2
		regex e4("SELECT\\s+[a-z0-9]+(,\\s*[a-z0-9]+)*\\sFROM\\s+[a-z0-9]+");
			//SELECT field1, fild2..ect FROM table
		regex e5("SELECT\\s+\\*\\s+FROM\\s+[a-z0-9]+\\s+WHERE\\s+[a-z0-9]+\\s*=\\s*(([-+]?[0-9]+)|(\"[a-zA-Z0-9\\s.\\-]+)\")");
			//SELECT * FROM table WHERE field=constant
		
		//*****************************************************************************
		//          COMMAND 1: CREATE table (fd1 INT, fd2 STRING, etc..)
		//*****************************************************************************
		if(regex_match(inputCmd, e1))
		{
			//Check if the table is already exist: If it's already exist, print out error msg
			//Else, create the table and update the dictionary
			
			splitString(inputCmd);
			found = checkTable(command[1]);//command[1] contains the table name
			if(found)
			{
				cout<<"\nTable already exist!"<<endl;
			}
			else
			{
				//1.update dictionary (in memory & disk), use table name as the address
				
				outfile.open("DataDictionary.txt", ios::app);
				for (int i = +2; i < command.size(); i=i+2)
				{
					tempTable.tablename = command[1];
					tempTable.fieldname = command[i];
					tempTable.fieldtype = command[i+1];
					tempTable.address = command[1];
					dictionary.push_back(tempTable);
					outfile<<tempTable.tablename<<" "<<tempTable.fieldname<<" "<<tempTable.fieldtype<<" "<<tempTable.address<<endl;
				}
				outfile.close();

				//2. create file and write to disk
				outfile.open(command[1]+".txt");
				outfile.close();
			}
		}

		//*****************************************************************************
		//              COMMAND 2: INSERT INTO table (value1,value2 etc..)
		//*****************************************************************************
		else if(regex_match(inputCmd, e2))
		{
			//1. extract table name, values, store in a string vector-"command"
			splitString_2(inputCmd);
			
			//2. check dictionary to see if table name is present. If not, print error msg
			//If found, check if the number of values matches the number of fields, if not print error msg
			//check if the values'types match the field types: if the field is INT, it must be numerical!
			
			found = checkTable(command[2]);//command[2] holds the table name
			if (!found)
			{
				cout<<"\nError: Table doesn't exist!"<<endl;
			}
			else //extract table info and check if the field name is correct
			{
				extractedTableInfo.clear();
				extractedTableInfo = extractTableInfoFromDic(command[2]);
				
				//check if the number of fields and types are correct.
				//write the values into a file, in the format: value1, value2, value3, ..

				bool isCorrect;
				isCorrect = checkField(command);
				if(isCorrect)
				{
					outfile.open(command[2]+".txt", ios::app);
					outfile<<command[3];
					if(extractedTableInfo.size() > 1)//if there are more than one fields
					{
						for (int i = 0; i < extractedTableInfo.size()-1; i++)
						{
							outfile<<", "<<command[i+4];
						}
					}

					outfile<<endl;
					outfile.close();
				}
			}
		}

		//*****************************************************************************
		//          COMMAND 3: SELECT * FROM table1, table2 WHERE fd1= fd2
		//*****************************************************************************
		else if(regex_match(inputCmd, e3))
		{
			int fieldCount1, fieldCount2;
			splitString(inputCmd);//parse the input command and store in string vector-"command"
								//position 3 & 4 contains table names, position 6 & 7 contains field names
			//check if both tables exist, if yes, read in an entry from table 1 and compare the field with
			//all the entries from table 2 (read in one entry at a time)

			if(checkTable(command[3])&&checkTable(command[4]))
			{
				extractedTableInfo.clear();
				extractedTableInfo = extractTableInfoFromDic(command[3]);
				
				extractedTableInfo2.clear();
				extractedTableInfo2 = extractTableInfoFromDic(command[4]);
				
				found = ( findItemInVectorOfStruct(extractedTableInfo, command[6])
						&& findItemInVectorOfStruct(extractedTableInfo2, command[7]));
			}
			if(!found)
			{
				cout<<"\nError: field not found!"<<endl;
			}
			else
			{
				int fieldCount1, fieldCount2;
				fieldCount1 = extractedTableInfo.size();
				fieldCount2 = extractedTableInfo2.size();
				infile.open(command[3]+".txt");
				

				while(!infile.eof())
				{
					infile2.open(command[4]+".txt");
					getline(infile, str1);
					map<string, string> entry1;
					entry1 = getTableEntry(str1, extractedTableInfo);
					while(!infile2.eof())
					{
						getline(infile2, str2);
						map<string, string> entry2;
						entry2 = getTableEntry(str2, extractedTableInfo2);
						
						if(entry1[command[6]].compare(entry2[command[7]])==0
							||(' '+entry1[command[6]]).compare(entry2[command[7]])==0
							||entry1[command[6]].compare(' '+entry2[command[7]])==0	)
						{
							//print the entry from table 1
							for(int i = 0; i < fieldCount1; i++)
							{
								cout<<entry1[extractedTableInfo[i].fieldname]<<" ";
							}
							//print the entry from table 2 except for the duplicated field
							for(int i = 0; i < fieldCount2; i++)
							{
								if(extractedTableInfo2[i].fieldname.compare(command[7])!=0)
									cout<<entry2[extractedTableInfo2[i].fieldname]<<" ";
							}
							cout<<endl;
						}
					}

					infile2.close();
					
				}
				infile.close();
			}
		}

		//*****************************************************************************
		//               COMMAND 4: SELECT fd {,fd} FROM table 
		//*****************************************************************************
		else if(regex_match(inputCmd, e4))
		{
			splitString(inputCmd);
			found = checkTable(command.back());//last item in command is the table name
			if (!found)
			{
				cout<<"\nError: Table doesn't exist!"<<endl;
			}
			else //extract table info and check if the field name is correct
			{
				extractedTableInfo.clear();
				extractedTableInfo = extractTableInfoFromDic(command.back());
				
				for (int i = 0; i < command.size()-3; i++)
				{
					
					found = findItemInVectorOfStruct(extractedTableInfo, command[i+1]);
					if(found ==0)
					{
						cout<<"\nError: type not found!"<<endl;
						break;
					}
				}
				
				if(found)//read in the attributes from table
				{
					infile.open(command.back()+".txt");
					while(!infile.eof())
					{
						getline(infile, str1);
						map<string, string> entry;
						entry = getTableEntry(str1, extractedTableInfo);

						for (int i = 1; i< command.size()-2; i++)
						{
							cout<<entry[command[i]]<<" ";
						}
						cout<<endl;
					}
					infile.close();
				}
			}
		}
		
		//*****************************************************************************
		//            COMMAND 5:SELECT * FROM table WHERE fd = constant 
		//*****************************************************************************
		else if(regex_match(inputCmd, e5))
		{
			string constant;
			constant =inputCmd.substr(inputCmd.find('=')+1);//get the constant starting with '"'
			
			inputCmd.erase(inputCmd.find('='), 50);
			
			splitString(inputCmd);//command holds: SELECT * FROM table WHERE fd
			
			found = checkTable(command[3]);//3rd item in command is the table name
			if (!found)
			{
				cout<<"\nError: Table doesn't exist!"<<endl;
			}
			else //extract table info and check if the field name is correct
			{
				extractedTableInfo.clear();
				extractedTableInfo = extractTableInfoFromDic(command[3]);
				
				if (!findItemInVectorOfStruct(extractedTableInfo, command[5]))
				{
					cout<<"\nError: Field not found!"<<endl;
				}
				else
				{
					infile.open(command[3]+".txt");
					while(!infile.eof())
					{
						getline(infile, str1);
						map<string, string> entry;

						entry = getTableEntry(str1, extractedTableInfo);
						if(constant.compare(entry[command[5]]) == 0 
							|| (' '+constant).compare(entry[command[5]]) == 0
							||constant.compare(' '+entry[command[5]]) == 0 )
						{
							for (int i = 0; i < extractedTableInfo.size(); i++)
							{
								cout <<entry[extractedTableInfo[i].fieldname]<<" ";
							}
						}
						cout<<endl;
					}
					infile.close();
				}
			}
		}
		else
		{
			cout<<"\nSyntax Error! Press Enter your command."<<endl;
			getline(cin, inputCmd);
			if(inputCmd =="EXIT")
			{
				repeat = false;
			}
		}
		
	}

	return 0;
}


//*************************************************************
//                     Function Implementations
//*************************************************************
void splitString(string str)
{
	
	string x;
	replace(str.begin(), str.end(), ',', ' ');
	replace(str.begin(), str.end(), '(', ' ');
	replace(str.begin(), str.end(), ')', ' ');
	replace(str.begin(), str.end(), '=', ' ');
	istringstream ss(str);
	command.clear();
	while(ss>>x)
	{
		command.push_back(x);
	}
	cout<<endl;
	return;
}
void splitString_2(string str)
{
	string substring, item;
	istringstream ss, ss1;
	
	substring = str.substr(str.find('(')+1);
	substring.pop_back(); //substring = value1, value2, value3
	
	str.erase(str.find('('), substring.length()+2); //str = INSERT INTO table
	
	ss.str(str);
	command.clear();
	while (ss>>item)
	{
		command.push_back(item);
	}
	
	ss1.str(substring);
	while(getline(ss1,item, ','))
	{
		if(item.front() == ' ')
		{
			item.erase(0, 1);
		}
		command.push_back(item);
	}	
	return;
}
bool checkTable(string tbName)
{
	auto it = find_if(dictionary.begin(), dictionary.end(),[tbName](tableinfo &item)
	{
		return item.tablename == tbName;
	});
	if (it != end(dictionary))
	{
		return true;
	}
	else
	{
		return false;
	}
}

vector<tableinfo> extractTableInfoFromDic(string tbName)
{
	vector<tableinfo> tbInfo;
	int count;
	count = dictionary.size();
	for(int i = 0; i < count; i++)
	{
		if(dictionary[i].tablename == tbName)
		{
			tbInfo.push_back(dictionary[i]);
		}
	}
	
	return tbInfo;
}

bool checkField(vector<string>& cmd)
{
	bool isFieldCorrect = true;
	int count1, count2;
	count1 = extractedTableInfo.size();
	count2 = cmd.size()-3;
	if(count1 != count2)
	{
		cout<<"\nThe number of fields in the input does not match the number of fields in the table."<<endl;
		isFieldCorrect = false;
	}
	else
	{
		regex e("[0-9]+");
		for (int i = 0; i < count1; i++)
		{
			if((extractedTableInfo[i].fieldtype == "INT")&&!(regex_match(cmd[i+3], e)))
			{
				cout<<"\nError: Data type does not match."<<endl;
				isFieldCorrect = false;
				break;
			}
		}
	}
	return isFieldCorrect;
}

bool findItemInVectorOfStruct(vector<tableinfo>& searchPool, string item)
{
	int poolSize = searchPool.size();
	bool isFound = 0;
	
	for(int i = 0; i < poolSize; i++)
	{
		if(searchPool[i].fieldname == item)
		{
			isFound =1;
			break;
		}
	}
	return isFound;
}

map<string, string> getTableEntry(string str, vector<tableinfo>& tbInfo)
{
	
	int fieldCount = tbInfo.size();
	string item;
	istringstream temp_ss(str);
	map<string, string> entry;
	for (int i = 0; i < fieldCount; i++)
	{
		getline(temp_ss, item, ',');
		
		entry[tbInfo[i].fieldname] = item;
	}

	return entry;
}
