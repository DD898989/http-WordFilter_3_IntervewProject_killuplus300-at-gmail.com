//#include "stdafx.h" 
#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
#include <codecvt>
#include <sstream>
#include <regex>
#include <iostream>
//---------------------------------------------------------------------------------------------------
using namespace std;
vector<string>  m_vCmd;
int m_nMaxBadWordLen = 0;
static const int SORTING_COUNT = 10;
vector<wstring>  m_vFilteredWord;
//---------------------------------------------------------------------------------------------------
inline int strcasecmp(const wchar_t *s1, const wchar_t *s2)  
{  
	int c1, c2;  
	do {  
		c1 =  *s1++;   //tolower(*s1++); //
		c2 =  *s2++;   //tolower(*s2++); //
	} while(c1 == c2 && c1 != 0);  //c1 != 0: is end of char[]?
	return c1 - c2;  
}  
//---------------------------------------------------------------------------------------------------
struct TreeNode 
{
	wchar_t word[128];
	struct TreeNode *left, *right;
};
//---------------------------------------------------------------------------------------------------
class DictionaryBinaryTree
{
public :
	struct TreeNode *root;
	struct TreeNode * createNode(wchar_t *word) 
	{
		struct TreeNode *newnode;

		newnode = (struct TreeNode *)malloc(sizeof(struct TreeNode));
		wcscpy(newnode->word, word);
		newnode->left = newnode->right = NULL;
		return newnode;
	}
	void Insert(wchar_t *word) 
	{
		struct TreeNode *parent= NULL, *current = NULL, *newnode = NULL;

		if (!root) 
		{
			root = createNode(word);
			return;
		}

		int res = 0;
		current = root;
		while(current != NULL)
		{
			res = strcasecmp(word, current->word);

			if (res == 0) 
				return;

			parent = current;

			if(res > 0)
				current = current->right;
			else
				current = current->left;
		}

		newnode = createNode(word);
		res > 0 ? (parent->right = newnode) : (parent->left = newnode);
	}

	bool Find(wchar_t *str) 
	{
		struct TreeNode *temp = NULL;
		int flag = 0, res = 0;
		temp = root;
		while (temp) 
		{
			if ((res = strcasecmp(temp->word, str)) == 0) 
				return true;

			temp = (res > 0) ? temp->left : temp->right;
		}
		return false;
	}
};
//---------------------------------------------------------------------------------------------------
DictionaryBinaryTree DBT;
//---------------------------------------------------------------------------------------------------
void AddCmd()
{
	m_vCmd.push_back("開始過濾");	m_vCmd.push_back("取代字詞");	m_vCmd.push_back("搜尋目標");	m_vCmd.push_back("新增目標");	m_vCmd.push_back("離開程式");
}
//---------------------------------------------------------------------------------------------------
enum ECommand
{
	E_RUN,							E_REPLACE_WORD,					E_SEARCH,						E_ADD,							E_EXIT,
};
//---------------------------------------------------------------------------------------------------
string wstring_to_utf8 (const wstring& str)
{
	wstring_convert<codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}
//---------------------------------------------------------------------------------------------------
wstring Read(string filename)
{
	wifstream wif(filename);
	wif.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));
	wstringstream wss;
	wss << wif.rdbuf();
	return wss.str();
}
//---------------------------------------------------------------------------------------------------
bool IsBadWord(wstring ws)
{
	wchar_t *str = new wchar_t[ws.length() + 1];	  wcscpy(str, ws.c_str());
	bool bResult = DBT.Find(str); 
	delete str;
	return bResult;
}
//---------------------------------------------------------------------------------------------------
void  Insert(wstring ws)
{
	wchar_t *str; str = new wchar_t[ws.length() + 1];	wcscpy(str, ws.c_str());
	DBT.Insert(str);
	delete str;
}
//---------------------------------------------------------------------------------------------------
void ReadBadWordDic(string sDicFile)
{
	m_vFilteredWord.clear();

	const locale empty_locale = locale::empty();
	typedef codecvt_utf8<wchar_t> converter_type;	
	const converter_type* converter = new converter_type;
	const locale utf8_locale = locale(empty_locale, converter);
	wifstream stream((sDicFile));
	stream.imbue(utf8_locale);
	wstring line;

	ifstream inFile(sDicFile); 
	int countline = count(istreambuf_iterator<char>(inFile), 
		istreambuf_iterator<char>(), '\n');
	m_vFilteredWord.resize(countline*3);

	clock_t t1 = clock();		/*///////////////////////////////////////////*/
	int index =-1;
	int count =0;
	while (getline(stream, line))
	{
		if(m_nMaxBadWordLen<line.length())
			m_nMaxBadWordLen = line.length();

		count++;
		if((countline-count)%5000==0)
			cout<<"Reading bad word dictionary, remaining:"<<countline-count<<endl;

		for(int i=0;i<line.length();i++) 
			line[i]= towupper(line[i]);
		m_vFilteredWord[++index]=line;

		for(int i=0;i<line.length();i++) 
			line[i]= towlower(line[i]);
		if(line!=m_vFilteredWord[index])
			m_vFilteredWord[++index]=line;

		line[0]= towupper(line[0]);
		if(line!=m_vFilteredWord[index])
			m_vFilteredWord[++index]=line;

	}
	clock_t t2 = clock();   	/*///////////////////////////////////////////*/  printf("%s:%f\n", "Reading Time: ",(t2-t1)/(double)(CLOCKS_PER_SEC));


	t1 = clock();		/*///////////////////////////////////////////*/
	m_vFilteredWord.erase( unique( m_vFilteredWord.begin(), m_vFilteredWord.end() ), m_vFilteredWord.end() );
	sort(m_vFilteredWord.begin(), m_vFilteredWord.end());
	int len0 = m_vFilteredWord.size();
	for(int k=2;k<m_vFilteredWord.size();k*=2)
	{
		int len = m_vFilteredWord.size();
		for(int j=k-1;j>0;j-=2)
		{
			int index = (double)len*((double)j/(double)k);
			if(index+1>m_vFilteredWord.size())
				break;

			if(m_vFilteredWord[index] == L"")
				continue;

			Insert(m_vFilteredWord[index]);
			m_vFilteredWord[index]=L"";
			
			if(--len0%5000==0)
				cout<<"Sorting bad word dictionary, remaining:"<<len0<<endl;
		}
	}

	vector<wstring>::iterator it;
	for(it=m_vFilteredWord.begin() ; it!=m_vFilteredWord.end() ; it++)
	{
		if(*it==L"")
			continue;

		Insert(*it);

		if(--len0%5000==0)
			cout<<"Sorting bad word dictionary, remaining: "<<len0<<endl;
	}

	m_vFilteredWord.clear();

	t2 = clock();   	/*///////////////////////////////////////////*/  printf("%s:%f\n", "Sorting Time: ",(t2-t1)/(double)(CLOCKS_PER_SEC));
}
//---------------------------------------------------------------------------------------------------
wstring wcinRegMatch(wstring sMatch)
{
	locale::global(locale(""));
	while(true)
	{
		wstring buffer;
		getline(wcin, buffer);
		wregex  rx(sMatch);
		wsmatch m;
		wstring str = buffer;
		if(regex_search(str, m, rx))
		{
			return buffer;
		}
	}
}
//---------------------------------------------------------------------------------------------------
int cinRegMatchWholeNumber(string sMatch)
{
	while(true)
	{
		string buffer;
		getline(cin, buffer);
		regex rx(sMatch);
		smatch m;
		string str = buffer;
		if(regex_search(str, m, rx))
		{
			int nNum;
			istringstream is(buffer);
			is>>nNum;
			return nNum;
		}
	}
}
//---------------------------------------------------------------------------------------------------
string ExePath() 
{
	char buffer[MAX_PATH];
	GetModuleFileNameA( NULL, buffer, MAX_PATH );
	string::size_type pos = string( buffer ).find_last_of( "\\/" );
	return string( buffer ).substr( 0, pos);
}
//---------------------------------------------------------------------------------------------------
int main()
{
	AddCmd();

	//global para
	wstring wsOutput;
	bool bRun = false;
	int cmd;
	ofstream myfile;

	//default value
	string sRootPath = ExePath()+"\\";
	string sOutputFile =sRootPath+"Output.txt";
	string sInputFile =sRootPath+"Input.txt";
	string sDicFile =sRootPath+"BadWords.txt";
	wstring wsReplace = L"【X】";

	//case para
	wstring wsRemove;
	wstring wsAdd;
	wstring wsSearch;

	locale::global(locale(""));
	cout<<"取代字詞:"; wcout<<wsReplace<<endl;
	cout<<"輸出路徑:"<<sOutputFile<<endl;
	cout<<"字典路徑:"<<sDicFile<<endl;

	cout<<"\n確認 字典路徑 檔案後按下enter繼續\n";
	system("pause");

	ReadBadWordDic(sDicFile);

	while(true)
	{
		cout<<endl<<endl;
		cout<<"請輸入CMD："<<endl;
		for(int i=0;i<m_vCmd.size();i++)
			cout<<i<<"."<<m_vCmd[i]<<" ";
		cout<<endl;
		cmd = cinRegMatchWholeNumber("^([0-9]*)$");
		cout<<endl<<endl;

		switch(cmd)//cmd //(int)E_RUN
		{
			//---------------------------------------------
		case (int)E_RUN:
			cout<<"開始過濾"<<endl;
			bRun=true;
			break;
			//---------------------------------------------
		case (int)E_REPLACE_WORD:
			cout<<"請輸入 "<<m_vCmd[cmd]<<" ："<<endl;
			wsReplace = wcinRegMatch(L"^.+$");
			break;
			//---------------------------------------------
		case (int)E_SEARCH:
			cout<<"請輸入 "<<m_vCmd[cmd]<<" ："<<endl;
			wsSearch = wcinRegMatch(L"^.+$");

			if(IsBadWord(wsSearch))
			{
				cout<<"找到"<<endl;
				break;
			}
			cout<<"沒找到"<<endl;
			break;
			//---------------------------------------------
		case (int)E_ADD:
			cout<<"請輸入 "<<m_vCmd[cmd]<<" ："<<endl;
			wsAdd = wcinRegMatch(L"^.+$");
			Insert(wsAdd);
			break;
			//---------------------------------------------
		case (int)E_EXIT:
			return 0;
			//---------------------------------------------
		default:
			break;
			//---------------------------------------------
		}
		//---------------------------------------------
		if(bRun)
		{
			bRun = false;
			wstring wsInput;// = Read(sInputFile); 
			cout<<endl<<"請輸入過濾句子:"<<endl;
			getline(wcin, wsInput);

			/*計時開始*/  static LARGE_INTEGER nFreq; static LARGE_INTEGER nBeginTime; static LARGE_INTEGER nEndTime; static int count=0;  static double timeT=0;  QueryPerformanceFrequency(&nFreq); QueryPerformanceCounter(&nBeginTime); //-----------------------------------------------------------------------------------

			int* ptr_nFilterRecord        = new int[wsInput.size()]();
			int* ptr_nFilterRecordForMark = new int[wsInput.size()]();

			for(int i =0;i<wsInput.length();i++)
			{
				int temp = wsInput[i];
				//各類符號跟標記
				if( temp==9  ||  temp==10  || temp==13 ||  (temp>=32 && temp<=47)|| (temp>=58 && temp<=64) || (temp>=91 && temp<=96) || (temp>=123 && temp<=126) ||
					temp==8739  || temp==65306 || temp==65307 || temp==8212  || temp==8231  || temp==65288 || temp==65289 || 
					temp==65292 || temp==65311 || temp==12290 || temp==65281 || temp==12301 || temp==12300 || temp==8217
					)
				{
					ptr_nFilterRecord[i] = 1;
					ptr_nFilterRecordForMark[i] = 1;
				}
			}

			int nSortCount = 0;
			for(int nWordLen=m_nMaxBadWordLen;nWordLen>0;nWordLen--)
			{
				int diff = wsInput.length()-nWordLen;//need assign to diff to check
				if(diff<0)
					continue;

				for(int i=0;i<wsInput.length()-nWordLen+1;i++)
				{
					if(ptr_nFilterRecord[i]>=nWordLen)
					{
						i+=(nWordLen-1);
						continue;
					}
					for(int j=i+nWordLen-1;j<i;j--)
					{
						if(ptr_nFilterRecordForMark[j]==1)
						{
							i=j;
							continue;
						}
					}

					wstring wsTemp;
					for(int j=0;j<nWordLen;j++)
						wsTemp += wsInput[i+j];
					if(IsBadWord(wsTemp))
					{
						for(int j=0;j<nWordLen;j++)
							ptr_nFilterRecord[i+j]=1;
						i += (nWordLen-1);

						if(nSortCount++ % SORTING_COUNT==0)
						{
							int pos = -1; 
							for(int k=0;k<wsInput.length();k++)
							{
								if(ptr_nFilterRecord[k]>0 &&  pos==-1)
									pos = k;

								if(ptr_nFilterRecord[k]==0 && pos>-1)
								{
									ptr_nFilterRecord[pos]=k-pos;
									pos = -1;
								}
							}
						}
					}
				}
			}

			//recover mark
			for(int i =0;i<wsInput.length();i++)
				if(ptr_nFilterRecordForMark[i] == 1)
					ptr_nFilterRecord[i] = 0;

			for(int i =0;i<wsInput.length();i++)
			{
				if(
					i>0&&i<wsInput.length()-1  &&  ptr_nFilterRecordForMark[i] == 1  
					&&  
					( (ptr_nFilterRecord[i-1] >0 )  && (ptr_nFilterRecord[i+1] > 0) )    
					)
				{
					ptr_nFilterRecord[i] = 1;
				}
			}


			//Special run length encode
			vector<int> vRunLengthEncode;
			int pos = -1;
			int len = wsInput.size();
			for (int j = 0; j < len; ++j)
			{
				if(ptr_nFilterRecord[j]>0 &&  pos==-1)
				{
					pos = j;
				}

				if((ptr_nFilterRecord[j]==0) && pos>-1)
				{
					vRunLengthEncode.push_back(pos);
					vRunLengthEncode.push_back(j-pos);
					pos=-1;
				}

				if(j==len-1)
				{
					if(ptr_nFilterRecord[j]>0)
					{
						vRunLengthEncode.push_back(pos);
						vRunLengthEncode.push_back(j-pos+1);
					}
				}
			}


			//replace
			wsOutput.assign( wsInput );
			for(int i=vRunLengthEncode.size()-2;i>-1;i-=2)
				wsOutput = wsOutput.replace(vRunLengthEncode[i], vRunLengthEncode[i+1], wsReplace);

			delete ptr_nFilterRecord;
			delete ptr_nFilterRecordForMark;
			/*計時結束*/  QueryPerformanceCounter(&nEndTime);    timeT+=(double)(nEndTime.QuadPart-nBeginTime.QuadPart)/(double)nFreq.QuadPart;count++;if(count%1==0){ printf("\nReplacing time:%f\n",timeT/count); timeT=0;count=0;}

			wcout<<endl<<L"過濾後:"<<endl<<wsOutput<<endl;;

			myfile.open (sOutputFile);
			myfile << wstring_to_utf8(wsOutput);
			myfile.close();
			//---------------------------------------------
		}
	}
	return 0;
}
//---------------------------------------------------------------------------------------------------
