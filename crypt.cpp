#include <iostream>
#include <string>
#include <cmath>
#include <time.h>
#include <random>
#include <chrono>
#include <algorithm>
#include <array>
#include <windows.h>
#include <stdio.h>

#define INFO_BUFFER_SIZE 32767

using namespace std;



// VALID CHARACTER TABLE - capital letters excluded per assignment(says a-z not A-Z) //
array<char,89> CHAR_TABLE = {'R', ';', 'K', '9', 'J', 'Q', '0', '!', '%', 'G', 'u', ':',
'z', '.', ']', 'M', '^', 'h', 'Z', ' ', '$', 'E', 'H', '5', 'V', 'P', '/\\', '8', '1',
'-', '3', 'v', 'U', 'e', 'd', '*', 'F', 'C', '7', 'f', 'x', 'T', '`', '+', 'c', ',', 'A',
'2', 'I', 'w', '6', ')[', 'W', 'p', '(', 'b', '4', 'i', 'l', 'g', 'k', 'D', '&', 'q', '=',
'B', '~', 'O', 'a', 'j', 'Y', 'y', '#', '@', 'm', 'S', 'X', 't', 'N', 's', 'r', 'L', 'n', 'o', '\'', '_',
'<', '>', '?'};

const unsigned MAX_USERS = 3;

// MAX COLUMNS FOR TRANSPOSITION CIPHER //
const int TRANSPOSITION_COLUMNS = 6;

array<string, MAX_USERS> users = {"tw07310", "tbowers", "tyler"};

bool userCheck()
{
    TCHAR infoBuf[INFO_BUFFER_SIZE];
    DWORD bufCharCount = INFO_BUFFER_SIZE;
    GetUserName(infoBuf, &bufCharCount);
    for (unsigned k = 0; k < MAX_USERS; k++)
    {
        if (infoBuf == users[k])
        {
            return true;
        }
    }
    return false;
}

void randomize()
{
    time_t seconds;
    seconds = time (NULL);
    int days = seconds/(60*60*24);
    unsigned seed = days;
    shuffle(CHAR_TABLE.begin(), CHAR_TABLE.end(), default_random_engine(seed));
}

std::string GetClipboardText()
{
  // Try opening the clipboard
  if (! OpenClipboard(nullptr))
    cout << "error" << endl;

  // Get handle of clipboard object for ANSI text
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == nullptr)
    cout << "error" << endl;

  // Lock the handle to get the actual text pointer
  char * pszText = static_cast<char*>( GlobalLock(hData) );
  if (pszText == nullptr)
    cout << "error" << endl;

  // Save text in a string class instance
  std::string text( pszText );

  // Release the lock
  GlobalUnlock( hData );

  // Release the clipboard
  CloseClipboard();

  return text;
}


void toClipboard(const std::string &s){
	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE,s.size() + 1);
	if (!hg){
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg),s.c_str(),s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT,hg);
	CloseClipboard();
	GlobalFree(hg);
}

// GET INDEX OF CHARACTER FROM CHAR_TABLE //
int lookup(char character)
{
    for (int k = 0; k < sizeof(CHAR_TABLE); k++)
    {
        if (character == CHAR_TABLE[k])
        {
            return k;
        }
    }
    return -1;
}

// TRANSPOSITION ENCRYPTION //
string transposition_encrypt(string msg)
{
    string result;
    int numOfRows = ceil(float(msg.length()) / TRANSPOSITION_COLUMNS);
    string * msgTable = new string[numOfRows];

    // KEEP TRACK OF WHICH ROW WE ARE FILLING //
    int currRow = 0;
    for (int k = 0; k < msg.length(); k += TRANSPOSITION_COLUMNS)
    {
        msgTable[currRow] = msg.substr(k, TRANSPOSITION_COLUMNS);
        currRow++;
    }

    // PLUG CHARACTERS INTO result STRING GOING [ROW X][COLUMN Y], [ROW X][COLUMN Y]
    for (int k = 0; k < TRANSPOSITION_COLUMNS; k++)
    {
        for (int j = 0; j < numOfRows; j++)
        {
            result += msgTable[j][k];
        }
    }

    // CLEAN UP //
    delete[] msgTable;

    return result;
}

// TRANSPOSITION DECRYPTION //
string transposition_decrypt(string msg)
{
    string result;
    int numOfColumns = ceil(float(msg.length()) / TRANSPOSITION_COLUMNS);
    string * msgTable = new string[TRANSPOSITION_COLUMNS];

    // KEEP TRACK OF WHICH ROW WE ARE FILLING //
    int currRow = 0;
    for (int k = 0; k < msg.length(); k += numOfColumns)
    {
        msgTable[currRow] = msg.substr(k, numOfColumns);
        currRow++;
    }

    // PLUG CHARACTERS INTO result STRING GOING [ROW X][COLUMN Y], [ROW X][COLUMN Y]
    for (int k = 0; k < numOfColumns; k++)
    {
        for (int j = 0; j < TRANSPOSITION_COLUMNS; j++)
        {
            result += msgTable[j][k];
        }
    }

    delete[] msgTable;

    return result;
}

// AUTOKEY ENCRYPTION //
string autokey_encrypt(string msg, string key)
{
    // BUILD KEY (KEY + MSG MINUS LAST CHARACTER) (i.e. MSG = 'ABC', KEY = 'T', KEY IS NOW 'TAB' //
    key += msg.substr(0, (msg.length() - 1));
    string result;

    // BUILD result BY TAKING CURRENT MSG AND KEY INDEX SUM AND MODDING BY CHAR_TABLE SIZE //
    // THIS SAVES US FROM MANUALLY TRACKING THROUGH A 2D ARRAY OF CHAR_TABLE FOR SIMULATION AUTOKEY ENCRYPTION BY HAND //
    for (int k = 0; k < msg.length(); k++)
    {
        result += CHAR_TABLE[((lookup(msg[k]) + lookup(key[k])) % sizeof(CHAR_TABLE))];
    }

    return result;
}

// AUTOKEY DECRYPTION //
string autokey_decrypt(string msg, string key)
{
    string result;
    for (int k = 0; k < msg.length(); k++)
    {
        // AGAIN, AVOIDING 2D ARRAY OF CHAR_TABLE FOR SIMULATION OF AUTOKEY ENCRYPTION BY HAND //
        // THIS GIVES THE SAME RESULTS BY USING INDEX DISTANCE //
        int x = sizeof(CHAR_TABLE) - lookup(key[0]);
        key = CHAR_TABLE[(lookup(msg[k]) + x) % sizeof(CHAR_TABLE)];
        result += key;
    }

    return result;
}

// ADDITIVE ENCRYPTION //
string additive_encrypt(string msg, int key)
{
    string result;
    // BUILD result BY SHIFTING CHARACTER INDEX by KEY AMOUNT. //
    for (int k = 0; k < msg.length(); k++)
    {
        result += CHAR_TABLE[(lookup(msg[k]) + key) % sizeof(CHAR_TABLE)];
    }

    return result;
}

// ADDITIVE DECRYPTION //
string additive_decrypt(string msg, int key)
{
    string result;
    // BUILD result BY SHIFTING CHARACTER INDEX by KEY AMOUNT. //
    // TERNARY IN CASE MSG[K] CHAR INDEX - KEY IS LESS THAN 0. DON'T WANT -5 % 31, SO INSTEAD -5 + 31 = 26 (i.e. 'a' with key 5 should be ' ' space) //
    for (int k = 0; k < msg.length(); k++)
    {
        result += CHAR_TABLE[((lookup(msg[k]) - key) >= 0) ? (lookup(msg[k]) - key) % sizeof(CHAR_TABLE) : (lookup(msg[k]) - key) + sizeof(CHAR_TABLE)];
    }

    return result;
}

// ENCRYPT UI //
string encrypt(string msg, int addkey, string autokey)
{

    while (msg.length() % 6 != 0)
    {
        msg += ' ';
    }
    msg = transposition_encrypt(msg);

    if (addkey > 0 && addkey < sizeof(CHAR_TABLE))
    {
        msg = additive_encrypt(msg, addkey);
    }
    else
    {
        cout << "invalid key: " << addkey << endl;
        return "";
    }

    if (lookup(autokey[0] != -1))
    {
        msg = autokey_encrypt(msg, autokey);
    }
    else
    {
        cout << "invalid key: " << autokey << endl;
        return "";
    }
    toClipboard(msg);
    return msg;
}

// DECRYPT UI //
string decrypt(string msg, int addkey, string autokey)
{
    if (lookup(autokey[0] != -1))
    {
        msg = autokey_decrypt(msg, autokey);
    }
    else
    {
        cout << "invalid key: " << autokey << endl;
        return "";
    }

    if (addkey > 0 && addkey < sizeof(CHAR_TABLE))
    {
        msg = additive_decrypt(msg, addkey);
    }
    else
    {
        cout << "invalid key: " << addkey << endl;
        return "";
    }

    msg = transposition_decrypt(msg);
    return msg;
}

// MAIN UI //

int main()
{
    if (userCheck())
    {
            randomize();
    }
    unsigned int option;
    string msg;

    cout << "Enter keys: ";
    int addkey;
    cin >> addkey;
    string autokey;
    cin >> autokey;
    cin.ignore();

    while (option != -1)
    {
        cout << "Encrypt(1) or Decrypt(0): ";
        cin >> option;
        if (option != 0 && option != 1)
        {
            cout << "invalid option" << endl;
            cin.ignore();
            cin.get();
            return -1;
        }
        else
        {
            if (option == 1)
            {
                cin.ignore();
                getline(cin, msg);
                msg = encrypt(msg, addkey, autokey);
            }
            else if (option == 0)
            {
                msg = GetClipboardText();
                unsigned start = 0;
                for (unsigned k = 0; k < msg.length(); k++)
                {
                    if (msg[k] == 13 || msg[k] == 10)
                    {
                        start++;
                    }
                    else
                    {
                        break;
                    }
                }
                msg = decrypt(msg.substr(start,msg.length()), addkey, autokey);
                cout << "\"" + msg + "\"" << endl;
            }
            else
            {
                cout << "Invalid option. Please enter 1 for Encryption or 0 for Decryption." << endl;
            }
        }
    }
    cin.ignore();
    cin.get();
    return 0;
}
