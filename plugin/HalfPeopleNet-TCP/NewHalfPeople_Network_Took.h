#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define MaxBuffSize 1024

namespace HalfPeopleTook
{
	namespace Network
	{
		namespace UDP
		{
		}
		namespace TCP
		{
			namespace Server
			{
				namespace SaveData
				{
					SOCKET sockSrv;
					int ConnectIndex = 0;
					int MaxConnect;
					void(*Error)(SOCKET* ClientSocket, const char* ErrorMessage);
					void (*Ms)(SOCKET* ClientSocket);
					void (*Log)(const char* Message);
					void (*Exit)(SOCKET* ClientSocket);
					SOCKADDR_IN addrClient;
					DWORD WINAPI ServerThread(LPVOID lpParameter)
					{
						SOCKET* ClientSocket = (SOCKET*)lpParameter;
						SOCKADDR_IN AddrClient = addrClient;

						Ms(ClientSocket);
						return 0;
					}
				}
				//initialization (int MaxConnect , int Potr , MessageJudgment(SOCKET* ClientSocket) , ErrorReceiving(SOCKET* ClientSocket, const char* ErrorMessage));
				void initialization(int MaxConnect, int Potr, void (*MessageJudgment)(SOCKET* ClientSocket), void(*ErrorReceiving)(SOCKET* ClientSocket, const char* ErrorMessage), void (*Exit)(SOCKET* ClientSocket))
				{
					SaveData::MaxConnect = MaxConnect;
					SaveData::Ms = MessageJudgment;
					SaveData::Error = ErrorReceiving;
					SaveData::Exit = Exit;
					// SaveData::Log =

					WSADATA wsaData;
					//SaveData::ClientID = -1;

					if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
					{
						//printf("Failed to load Winsock");
						SaveData::Error(NULL, "Failed to load Winsock");
						return;
					}

					//创建用于监听的套接字
					SaveData::sockSrv = socket(AF_INET, SOCK_STREAM, 0);

					SOCKADDR_IN addrSrv;
					addrSrv.sin_family = AF_INET;
					addrSrv.sin_port = htons(Potr); //1024以上的端口号
					addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

					int retVal = bind(SaveData::sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
					if (retVal == SOCKET_ERROR) {
						//printf("Failed bind:%d\n", WSAGetLastError());
						SaveData::Error(NULL, "Failed bind:%d" + WSAGetLastError());
						return;
					}

					if (listen(SaveData::sockSrv, 10) == SOCKET_ERROR) {
						//printf("Listen failed:%d", WSAGetLastError());
						SaveData::Error(NULL, "Listen failed:%d" + WSAGetLastError());
						return;
					}
				}

				void WaitingClientConnect(void(*ClientEnters)(SOCKET* ClientSocket))
				{
					int len = sizeof(SOCKADDR);
					while (true)
					{
						if (SaveData::ConnectIndex < SaveData::MaxConnect)
						{
							SOCKET* ClientSocket = new SOCKET;
							HANDLE hThread;
							int SockAddrlen = sizeof(sockaddr);
							*ClientSocket = accept(SaveData::sockSrv, (SOCKADDR*)&SaveData::addrClient, &len);
							if (*ClientSocket == SOCKET_ERROR) {
								//printf("\nAccept failed:%d", WSAGetLastError());
								SaveData::Error(ClientSocket, "\nAccept failed:%d" + WSAGetLastError());
							}
							SaveData::ConnectIndex++;
							ClientEnters(ClientSocket);
							hThread = CreateThread(NULL, NULL, &SaveData::ServerThread, (LPVOID)ClientSocket, 0, NULL);
							CloseHandle(hThread);
						}
						else
						{
							SOCKET ClientSocketSave = accept(SaveData::sockSrv, 0, 0);
							if (ClientSocketSave == SOCKET_ERROR) {
								//printf("\nAccept failed:%d", WSAGetLastError());
								SaveData::Error(NULL, "\nAccept failed:%d" + WSAGetLastError());
							}
							const char* TextSave = "8758932225464343533";
							send(ClientSocketSave, TextSave, 40, 0);
						}

						//						int len = sizeof(SOCKADDR);
						//						//等待客户请求到来
						//						SaveData::sockConn[SaveData::ClientID] = accept(SaveData::sockSrv, (SOCKADDR*)&SaveData::addrClient, &len);
						//						if (SaveData::sockConn[SaveData::ClientID] == SOCKET_ERROR) {
						//							//printf("\nAccept failed:%d", WSAGetLastError());
						//							SaveData::Error(SaveData::ClientID, "\nAccept failed:%d" + WSAGetLastError());
						//						}
						//						else
						//						{
						//							ClientEnters(SaveData::ClientID);
						//							//printf("\nAccept client IP:[%s]\n", inet_ntoa(addrClient.sin_addr));
						//
						//							SaveData::hThread[SaveData::ClientID] = CreateThread(NULL, 0, SandAndGetMs, (LPVOID)SaveData::ClientID, 0, 0);
						//							std::cout << "\n awdadadawd : " << tta;
						//
						//							tta++;
						//							SaveData::ClientID = tta;
					}
				}

				void Disconnect(SOCKET* ClientSocket)
				{
					SaveData::ConnectIndex--;
					//send(*ClientSocket, "Your server has been closed", MaxBuffSize, 0);
					closesocket(*ClientSocket);
					SaveData::Exit(ClientSocket);
					return;
				}

				//Exit the thread if wrong
				bool Receive(SOCKET* ClientSocket, char* ReceiveText)
				{
					int i = recv(*ClientSocket, ReceiveText, MaxBuffSize, 0);
					if (i < 0)
					{
						if (WSAGetLastError() == 10054)//檢測到客戶端主動關閉連接
							Disconnect(ClientSocket);
						else
							SaveData::Error(ClientSocket, "failed to receive,Error:" + WSAGetLastError());
						return false;
					}
					else
						return true;
				}
				//Exit the thread if wrong
				bool Send(SOCKET* ClientSocket, const char* SendText)
				{
					int i = send(*ClientSocket, SendText, MaxBuffSize, 0);
					if (i < 0)
					{
						if (WSAGetLastError() == 10054)//檢測到客戶端主動關閉連接
							Disconnect(ClientSocket);
						else
							SaveData::Error(ClientSocket, "failed to receive,Error:" + WSAGetLastError());
						return false;
					}
					else
						return true;
				}

				void ExitThread(SOCKET* ClientSocket)
				{
					if (*ClientSocket != INVALID_SOCKET)
					{
						closesocket(*ClientSocket);
					}
				}

				char* GetIP_Addr(SOCKADDR_IN Client)
				{
					return inet_ntoa(Client.sin_addr);
				}
			}

			namespace Client
			{
				namespace SaveData
				{
					SOCKADDR_IN addrSrv;
					SOCKET sockClient;
					void (*ER)(const char* ErrorMessage);
				}
				void initialization(int Port, const char* IP_Addr, void (*ErrorReceiving)(const char* ErrorMessage))
				{
#if Port==0
					Port = 5099;
#endif
					SaveData::ER = ErrorReceiving;
					WSADATA wsaData;
					char buff[1024];
					//memset(buff, 0, sizeof(buff));

					if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
					{
						SaveData::ER("\nFailed to load Winsock");
						return;
					}

					SaveData::addrSrv.sin_family = AF_INET;
					SaveData::addrSrv.sin_port = htons(Port);
					SaveData::addrSrv.sin_addr.S_un.S_addr = inet_addr(IP_Addr);

					SaveData::sockClient = socket(AF_INET, SOCK_STREAM, 0);
					if (SOCKET_ERROR == SaveData::sockClient) {
						//printf("\nSocket() error:%d", WSAGetLastError());
						SaveData::ER("\nSocket() error:%d" + WSAGetLastError());
						return;
					}
					if (connect(SaveData::sockClient, (struct  sockaddr*)&SaveData::addrSrv, sizeof(SaveData::addrSrv)) == INVALID_SOCKET) {
						SaveData::ER("\nConnect failed:%d" + WSAGetLastError());
						return;
					}
					return;
				}

				void Send(const char* SendText)
				{
					send(SaveData::sockClient, SendText, strlen(SendText) + 1, 0);
				}
				void Receive(char* ReceiveText, int Size)
				{
					recv(SaveData::sockClient, ReceiveText, Size, 0);
				}
				void Disconnect(bool CloseAll)
				{
					closesocket(SaveData::sockClient);
					if (CloseAll)
						WSACleanup();
				}
			}
		}
	}
}
