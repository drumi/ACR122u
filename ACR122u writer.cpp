#include "pch.h"
#include <winscard.h>
#pragma comment(lib, "winscard.lib")
#include <iostream>
#include<string>
#include<iomanip>

// variables
LPSTR szReaders = NULL;
LPCSTR readerName;
SCARDCONTEXT hContext = NULL;
SCARDHANDLE hCardHandle = NULL;
DWORD dwAP = SCARD_PROTOCOL_T1;
DWORD dwSend = 0;
DWORD dwRecv = 255;
LPBYTE pbSend;
LPBYTE pbRecv = new BYTE[255];
SCARD_IO_REQUEST pioSendPci;
SCARD_IO_REQUEST pioReceivePci;
SCARD_READERSTATEA reader;

const int sleepTime = 50;
bool isOkToPress;
std::string rName;

// functions
std::string getReaderName();

void waitForCardRemove(DWORD sleepTime, int previousState);

int connectCard();

void pressKeys(std::string input);

void printReceived(LPBYTE pbRecv, DWORD dwRecv);

void loadKey();

void waitForCard(DWORD sleepTime, int &previousstate);

std::string readSector(BYTE sector);

void writeSector(BYTE sector, std::string text);

// APDU commands
BYTE loadMifareDefaultKey[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };

BYTE AuthenticateBlock[] = { 0xFF, 0x86, 0x00, 0x00, 0x05, 0x01, 0x00, 0x01,// Block number in Hex to be authenthicated
							0x60, 0x00 };

BYTE ReadBlock[] = { 0xFF, 0xB0, 0x00, 0x01, // Block number in Hex
					0x10 };

BYTE WriteBlock[] = { 0xFF, 0xD6, 0x00, 0x01, // block number in Hex
					0x10, // length of data
					0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x05 }; // Data to write

int main()
{
	std::string userDetails;
	pioReceivePci.dwProtocol = SCARD_PROTOCOL_T1;
	pioReceivePci.cbPciLength = 1000;
	rName = getReaderName();
	readerName = rName.c_str();
	reader.szReader = readerName;

	LONG status = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
	if (status != SCARD_S_SUCCESS) {
		std::cout << "Contex error! \n";
		return -1;
	}

	//execution loop

	char pass[] = "1234567890";
	std::string passTry = "";
	std::cout << "Enter password: ";
	std::cin >> passTry;

	if (passTry.length() != 10)
		return -22;

	for (size_t i = 0; i < 10; i++)
	{
		if (passTry[i] != pass[i])
			return -23;
	}
	
	system("cls");
	std::cout << "Stop reading program before write attempts!\nPlace card on reader before writing!\n\n";
	std::cout << "Write options: \n\t1.User1\n\t2.User2\n\t3.User3\n\t4.Admin\n>";
	char ch;
	std::cin >> ch;

	int previousState;
	waitForCard(sleepTime, previousState);
	connectCard();
	loadKey();
	
	switch (ch)
	{
		case '1':
			std::cout << "Writing...\n";
			writeSector(2, "Whatever");
			std::cout << "Write result: " << std::boolalpha << (readSector(2) == "Whatever");
			Sleep(3000);
			break;

		case '2':
			std::cout << "Writing...\n";
			writeSector(2, "Whatever");
			std::cout << "Write result: " << std::boolalpha << (readSector(2) == "Whatever");
			Sleep(3000);
			break;

		case '3':
			std::cout << "Writing...\n";
			writeSector(2, "Whatever");
			std::cout << "Write result: " << std::boolalpha << (readSector(2) == "Whatever");
			Sleep(3000);
			break;

		case '4':
			std::cout << "Writing...\n";
			writeSector(2, "Whatever");
			std::cout << "Write result: " << std::boolalpha << (readSector(2) == "Whatever");
			Sleep(3000);
			break;

		default:
			std::cout << "Invalid input!";
			Sleep(3000);
			break;
	}

	//end resource managment
	SCardDisconnect(hCardHandle, 0);
	SCardReleaseContext(hContext);
}

void printReceived(LPBYTE pbRecv, DWORD dwRecv)
{
	for (size_t i = 0; i < dwRecv; i++)
	{
		std::cout << (int)pbRecv[i] << " ";
	}
	std::cout << std::endl;
}

void loadKey()
{
	dwRecv = 255;
	pbSend = loadMifareDefaultKey;
	SCardTransmit(hCardHandle, &pioSendPci, pbSend, 5, NULL, pbRecv, &dwRecv);
}

std::string readSector(BYTE sector)
{
	std::string result = "";

	dwRecv = 255;
	pbSend = AuthenticateBlock;
	AuthenticateBlock[7] = 0x01 * sector * 4;
	int status = SCardTransmit(hCardHandle, &pioSendPci, pbSend, 10, NULL, pbRecv, &dwRecv);
	if (status != SCARD_S_SUCCESS) 
	{
		std::cout << "Authentication Error\n";
		isOkToPress = false;
	}
	for (size_t i = 0; i <= 2; i++)
	{
		dwRecv = 255;
		pbSend = ReadBlock;
		ReadBlock[3] = sector * (BYTE)4 + (BYTE)i;
		status = SCardTransmit(hCardHandle, &pioSendPci, pbSend, 5, &pioReceivePci, pbRecv, &dwRecv);
		if (status != SCARD_S_SUCCESS) 
		{
			std::cout << "Read Error: " << status << "\n";
			isOkToPress = false;
		}
		for (size_t j = 0; j < dwRecv - 2; j++)
		{
			result += pbRecv[j];
		}
	}

	return result;
}

void writeSector(BYTE sector, std::string text)
{
	int length = strlen(text.c_str());
	dwRecv = 255;
	pbSend = AuthenticateBlock;
	AuthenticateBlock[7] = 0x01 * sector * 4;
	int status = SCardTransmit(hCardHandle, &pioSendPci, pbSend, 10, NULL, pbRecv, &dwRecv);
	if (status != SCARD_S_SUCCESS)
		std::cout << "Authentication Error\n";

	for (size_t i = 0; i <= 2; i++)
	{
		dwRecv = 255;
		pbSend = WriteBlock;
		WriteBlock[3] = sector * 4 + i;
		for (size_t j = 5; j <= 20; j++)
		{
			if (length >= (j - 5) + i * 16)
				text += '\0';
			WriteBlock[j] = text[(j - 5) + i * 16];
		}
		status = SCardTransmit(hCardHandle, &pioSendPci, pbSend, 21, NULL, pbRecv, &dwRecv);
		if (status != SCARD_S_SUCCESS)
			std::cout << "Write Error\n";
	}
}

void waitForCard(DWORD sleepTime, int &previousState)
{
	SCardGetStatusChangeA(hContext, INFINITE, &reader, 1);
	while (!(reader.dwEventState & SCARD_STATE_PRESENT))
	{
		Sleep(sleepTime);
		SCardGetStatusChangeA(hContext, INFINITE, &reader, 1);
	}
	previousState = reader.dwEventState;
}

void waitForCardRemove(DWORD sleepTime,int previousState)
{
	SCardGetStatusChangeA(hContext, INFINITE, &reader, 1);
	while (reader.dwEventState == previousState)
	{
		Sleep(sleepTime);
		SCardGetStatusChangeA(hContext, INFINITE, &reader, 1);
	}
}

int connectCard()
{
	LONG lReturn = SCardConnectA(hContext,
		readerName,
		SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		&hCardHandle,
		&dwAP);

	if (SCARD_S_SUCCESS != lReturn)
	{
		std::cout << "Failed SCardConnect\nError: " << lReturn << '\n';
		isOkToPress = false;
		return -2;
	}


	switch (dwAP)
	{
	case SCARD_PROTOCOL_T0:
		pioSendPci = *SCARD_PCI_T0;
		break;

	case SCARD_PROTOCOL_T1:
		pioSendPci = *SCARD_PCI_T1;
		break;
	default:
		std::cout << "Track error";
		return -3;
	}
}

void pressKeys(std::string input)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	
	for (size_t i = 0; i < input.length(); i++)
	{
		ip.ki.wVk = input[i]; 
		ip.ki.dwFlags = 0; 
		if (input[i] == '\n')
		{
			ip.ki.wVk = VK_RETURN;
		}

		SendInput(1, &ip, sizeof(INPUT));

		// Release the key
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}

	ip.ki.wVk = VK_RETURN;
	ip.ki.dwFlags = 0; // 0 for key press

	SendInput(1, &ip, sizeof(INPUT));

	// Release the key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

std::string getReaderName()
{
	DWORD dwReaders = SCARD_AUTOALLOCATE;

	do
		SCardListReadersA(hContext, NULL, (LPSTR)&szReaders, &dwReaders) == SCARD_S_SUCCESS;
	while (szReaders == '\0' && szReaders == NULL);

	std::string name = szReaders;
	return name;
}
