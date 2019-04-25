
// CodMakeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CodMake.h"
#include "CodMakeDlg.h"
#include "afxdialogex.h"
#include "beaengine/beaengine.h"
#include "Elib.h"
#include <string>
#include <math.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




void ReadHex(ULONG addr,int len,char* hextext) {   //�����ı���ʽ  
	byte *buffer = new byte[len];
	memset(buffer, 0, len);
	ReadProcessMemory((HANDLE)-1, (LPCVOID)addr, buffer, len, NULL);
	for (int n = 0;n < len;n++) {
		sprintf(hextext+n*2, "%02X", buffer[n]);
	}
	delete[]buffer;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCodMakeDlg �Ի���



CCodMakeDlg::CCodMakeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCodMakeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCodMakeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_output);
	DDX_Control(pDX, IDC_STATIC1, m_libname);
}

BEGIN_MESSAGE_MAP(CCodMakeDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CCodMakeDlg ��Ϣ�������

BOOL CCodMakeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	

	char bufBeaInfo[128] = { 0 };
	wsprintfA(bufBeaInfo, "CodeMake3.0  BeaEngineVersion = %s",BeaEngineVersion());

	SetWindowTextA(m_hWnd,bufBeaInfo);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CCodMakeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCodMakeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CCodMakeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


#ifndef IMAGE_SIZEOF_BASE_RELOCATION
// Vista SDKs no longer define IMAGE_SIZEOF_BASE_RELOCATION!?
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

static inline void*
OffsetPointer(void* data, ptrdiff_t offset) {
	return (void*)((uintptr_t)data + offset);
}

INT CCodMakeDlg::IsRelocated(ULONG addr,int len) {    //-1��ʾ���ض�λ����,�����ض�λƫ���ֽ�
	for (UINT n = 0;n < len - 3;n++) {
		if (m_relocation[addr + n] == true) {
			return n;
		}
	}
	return -1;
}



BOOL CCodMakeDlg::GenerateRelocationMap(unsigned char* pdll, ptrdiff_t delta)  //�����ض�λ��ַ��
{
	unsigned char *codeBase = pdll;
	PIMAGE_BASE_RELOCATION relocation;
	PIMAGE_DOS_HEADER pDosHead = (PIMAGE_DOS_HEADER)pdll;
	PIMAGE_NT_HEADERS pNtHead = (PIMAGE_NT_HEADERS)(pdll + pDosHead->e_lfanew);//NTͷ

	int nSection = pNtHead->FileHeader.NumberOfSections;
	DWORD tmp = (DWORD)pNtHead + sizeof(IMAGE_FILE_HEADER) + pNtHead->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_NT_SIGNATURE);
	PIMAGE_SECTION_HEADER	pSection = (PIMAGE_SECTION_HEADER)tmp;//����
	DWORD	dwExeSectionSize;
	for (int i = 0; i < nSection; i++)
	{
		if (pSection->Characteristics & PAGE_EXECUTE_READ)
		{
			dwExeSectionSize = (DWORD)(codeBase + pSection->Misc.VirtualSize);
		}
	}

	PIMAGE_DATA_DIRECTORY directory = (PIMAGE_DATA_DIRECTORY)&pNtHead->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	DWORD	dwIatStart = pNtHead->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress + (DWORD)codeBase;
	DWORD	dwIatEnd = dwIatStart + pNtHead->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;

	if (directory->Size == 0) {
		return (delta == 0);
	}

	relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
	for (; relocation->VirtualAddress > 0;) {
		DWORD i;
		unsigned char *dest = codeBase + relocation->VirtualAddress;
		unsigned short *relInfo = (unsigned short*)OffsetPointer(relocation, IMAGE_SIZEOF_BASE_RELOCATION);
		for (i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
			// the upper 4 bits define the type of relocation
			int type = *relInfo >> 12;
			// the lower 12 bits define the offset
			int offset = *relInfo & 0xfff;

			switch (type)
			{
			case IMAGE_REL_BASED_ABSOLUTE:
				// skip relocation
				break;
			case IMAGE_REL_BASED_HIGHLOW:
				// change complete 32 bit address
			{
				ULONG *patchAddrHL = (DWORD *)(dest + offset);
				DWORD dwOldProtect;
				VirtualProtect(patchAddrHL, sizeof(void*), PAGE_EXECUTE_READWRITE, &dwOldProtect);
				m_relocation[(ULONG)patchAddrHL] = true;
			}
			break;

#ifdef _WIN64
			case IMAGE_REL_BASED_DIR64:
			{
				ULONGLONG *patchAddr64 = (ULONGLONG *)(dest + offset);
				*patchAddr64 += (ULONGLONG)delta;
			}
			break;
#endif
			default:
				//printf("Unknown relocation: %d\n", type);
				break;
			}
		}

		// advance to next relocation block
		relocation = (PIMAGE_BASE_RELOCATION)OffsetPointer(relocation, relocation->SizeOfBlock);
	}
	return TRUE;
}

BOOL CCodMakeDlg::GenerateIATMap(unsigned char* pdll) {

	PIMAGE_DOS_HEADER pDosHead = (PIMAGE_DOS_HEADER)pdll;
	PIMAGE_NT_HEADERS pNtHead = (PIMAGE_NT_HEADERS)(pdll + pDosHead->e_lfanew);//NTͷ

	ULONG importoffset = pNtHead->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (importoffset == NULL) {
		return false;
	}
	PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(importoffset + (ULONG)pdll);     //��ȡ�����
	
	for (; importDesc->OriginalFirstThunk > 0;) {
		string dllname = (char*)(importDesc->Name + (ULONG)pdll);
		dllname.erase(dllname.find_first_of(".", 0)+1);
		ULONG* Funcoffset = (ULONG*)(importDesc->OriginalFirstThunk + pdll);
		ULONG FuncAddr = importDesc->FirstThunk + (ULONG)pdll;
		for (; *(ULONG*)Funcoffset> 0;) {
			if (*Funcoffset <= 0x70000000) {   //ƫ��Ӧ�ں���Χ��
				m_import[FuncAddr] = dllname + (char*)(*Funcoffset + (ULONG)pdll + 2);   //����IAT��
			}
			Funcoffset++;
			FuncAddr = FuncAddr + 4;
		}		
		importDesc++;
	}

	return true;
}

INT CCodMakeDlg::Findprocend(ULONG FuncStartAddr,string& FuncText) {   //�����ӳ����ַ,����ֵ��Ӧ��ͬ�ķ������,1�����������
	DISASM asmcode = {0};
	asmcode.EIP = FuncStartAddr;

	CallLevel = CallLevel + 1; 

	static char temp[32];

	ULONG MaxAddr = 0;

	for (;;) {
		int len = Disasm(&asmcode);

		if (len <= 0) {
			break;
		}

		for (int n = 0;n < len;n++) {
			m_block[asmcode.EIP + n] = true;    //�Ѿ��������Ĵ����
		}

		if (m_relocation[asmcode.EIP]) {	//��һ���ֽھ����ض�λ,���ǲ����ܵ�,˵�������ݵ������봦����
			FuncText.append("????????");
			asmcode.EIP = asmcode.EIP + 4;
			continue;
		}

		if (len >= 5) {					//�����볤�ȴ��ڵ���5��ָ��������Ǹ���ָ��,����ָ���

			int offset = IsRelocated(asmcode.EIP, len);		 //�ж��Ƿ�����ض�λ
			if (offset != -1) {								 //�����ض�λ��ģ�������ض�λ�ֽ�
				if (strcmp(asmcode.Instruction.Mnemonic,"jmp ")==0 && !m_import[asmcode.Argument1.Memory.Displacement].empty()) {        //jmp����IAT��ת
					FuncText.append("[" + m_import[asmcode.Argument1.Memory.Displacement] + "]");
					CallLevel = CallLevel - 1;
					return 1;
				}
				if (strcmp(asmcode.Instruction.Mnemonic, "call ") == 0 && !m_import[asmcode.Argument1.Memory.Displacement].empty()) {	 //call����IAT��ת
					FuncText.append("<[" + m_import[asmcode.Argument1.Memory.Displacement] + "]>");
					asmcode.EIP = asmcode.EIP + len;
					continue;
				}

				ReadHex(asmcode.EIP, len, temp);
				if (len>=9 && m_relocation[asmcode.EIP + offset+4]) {  //����˫�ض�λ
					for (int n = 0;n < 16;n++) {
						temp[(offset * 2) + n] = 0x3F;
					}
				}
				else
				{
					for (int n = 0;n < 8;n++) {
						temp[(offset * 2) + n] = 0x3F;
					}
				}
				
				FuncText.append(temp);
				asmcode.EIP = asmcode.EIP + len;
				continue;
			}
			else if (asmcode.Instruction.BranchType != 0 && asmcode.Instruction.AddrValue != 0) {		//jmp,je,callһ��ָ��,������ת��ַ
				if (asmcode.Instruction.BranchType == 11) {		//��jmpָ��,���� jmp $+100E
					if ((FuncStartAddr > asmcode.Instruction.AddrValue || asmcode.Instruction.AddrValue > MaxAddr) && m_block[asmcode.Instruction.AddrValue] == false && abs((long)(asmcode.Instruction.AddrValue-asmcode.EIP))>5) {
						if (asmcode.Instruction.AddrValue < (ULONG)hLib || asmcode.Instruction.AddrValue >= (ULONG)hLib + SectionSize) {  //�쳣������VMP����?
							FuncText.append("E9????????");
							CallLevel = CallLevel - 1;
							return 1;
						}
						FuncText.append("-->");
						MaxAddr = 0;
						asmcode.EIP = asmcode.Instruction.AddrValue;
					}
					else
					{
						ReadHex(asmcode.EIP, len, temp);
						FuncText.append(temp);
						asmcode.EIP = asmcode.EIP + len;
					}
					continue;
				}
				else if (asmcode.Instruction.BranchType == 12 && asmcode.Instruction.Opcode == 232)		//E8 callָ��,���� call 0x401000
				{
					if (CallLevel >= CALL_LEVEL){  //Ĭ���޶�Ϊ3��,����3���CALL������׷��,ȫ����ΪE8 ?? ?? ?? ??
						FuncText.append("E8????????");
						asmcode.EIP = asmcode.EIP + len;
						continue;
					}
					string calladdr = to_string(asmcode.Instruction.AddrValue);
					if (!m_call[calladdr]) {  //��һ��ʶ��ú���
						string calltext;
						m_call[calladdr] = true;                 //��ֹ�ݹ�
						Findprocend(asmcode.Instruction.AddrValue, calltext);
						WriteFile(hFile, (calladdr + ":" + calltext + "\r\n").c_str(), calladdr.length() + 3 + calltext.length(), &dwWritten, NULL);
					}
					FuncText.append("<" + calladdr + ">");
					asmcode.EIP = asmcode.EIP + len;
					continue;
				}
				else   //��ָ֧��,���� je $+100E
				{
					MaxAddr = max(MaxAddr, asmcode.Instruction.AddrValue);
					ReadHex(asmcode.EIP, len, temp);
					FuncText.append(temp);
					asmcode.EIP = asmcode.EIP + len;
					continue;
				}
			}
			else
			{
				ReadHex(asmcode.EIP, len, temp);
				FuncText.append(temp);
				asmcode.EIP = asmcode.EIP + len;
				continue;
			}
		}
		else {					//�����볤��С��5
			if (asmcode.Instruction.BranchType != 0 && asmcode.Instruction.AddrValue != 0) {		//����jmp,jeһ��ָ��,������ת��ַ
				if (asmcode.Instruction.BranchType == 11 && asmcode.EIP>=MaxAddr) {    //�еĺ�����jmpָ��Ϊ�ս�
					ReadHex(asmcode.EIP, len, temp);
					FuncText.append(temp);
					CallLevel = CallLevel - 1;
					return 1;
				}
				MaxAddr = max(MaxAddr, asmcode.Instruction.AddrValue);
				ReadHex(asmcode.EIP, len, temp);
				FuncText.append(temp);
				asmcode.EIP = asmcode.EIP + len;
				continue;
			}
			else if (asmcode.Instruction.BranchType == 13 && asmcode.EIP >= MaxAddr) {
				ReadHex(asmcode.EIP, len, temp);
				FuncText.append(temp);
				CallLevel = CallLevel - 1;
				return 1;
			}
			else
			{
				ReadHex(asmcode.EIP, len, temp);
				FuncText.append(temp);
				asmcode.EIP = asmcode.EIP + len;
				continue;
			}
		}
	}

	


	return 0;
}


bool MatchCode(unsigned char *pSrc1,unsigned char *pSrc2,int nLen)
{
	for (int i = 0; i < nLen; i++)
	{
		if (pSrc1[i] != pSrc2[i])
			return false;
	}
	return true;
}

void CCodMakeDlg::DebugMessage(char *format, ...)
{
	USES_CONVERSION;
	char buf[MAX_PATH] = { 0 };
	va_list st;
	va_start(st, format);
	vsprintf_s(buf, format, st);
	va_end(st);
	m_output.SetCurSel(m_output.InsertString(-1, A2W(buf)));
}


void CCodMakeDlg::MakeCode(LPWSTR lpFile)
{	
	USES_CONVERSION;
	CString strLibName,strLibVer,strGuid,strComCount;
	CString	strFile(lpFile);
	if (strFile.Right(4).CompareNoCase(L".fne") != 0)
	{
		AfxMessageBox(L"������ .fne ������֧�ֿ��ļ�!");
		return;
	}

	hLib = LoadLibrary(strFile);
	if (!hLib)
	{
		AfxMessageBox(L"����DLL�ļ�ʧ��!");
		return;
	}

	MEMORY_BASIC_INFORMATION MB;
	if (VirtualQuery(hLib+4096, &MB, sizeof(MB)) == 0) {
		AfxMessageBox(L"��ѯҳ��ʧ��!");
		return;
	}
	SectionSize = MB.RegionSize;

	typedef PLIB_INFO(WINAPI* FnGetNewInf)(void);
	FnGetNewInf pFnGetNewInf = (FnGetNewInf)GetProcAddress(hLib, "GetNewInf");
	if (!pFnGetNewInf)
	{
		AfxMessageBox(L"�����ӿ� GetNewInf ������!");
		return;
	}

	PLIB_INFO pLibInfo = pFnGetNewInf();
	
	strLibName.Format(L"֧�ֿ�����: %s", A2W((char*)pLibInfo->m_szName));
	SetDlgItemTextW(IDC_STATIC1, strLibName);

	strLibVer.Format(L"%1d.%1d", pLibInfo->m_nMajorVersion,pLibInfo->m_nMinorVersion);
	SetDlgItemTextW(IDC_STATIC2, (CString)L"�汾:" + strLibVer);

	strComCount.Format(L"��������: %d", pLibInfo->m_nCmdCount);
	SetDlgItemTextW(IDC_STATIC5, strComCount);

	strGuid.Format(L"%s", A2W((char*)pLibInfo->m_szGuid));
	SetDlgItemTextW(IDC_STATIC3, (CString)L"GUID: " + strGuid);

	//������������������������������������������������������������������������

	vector<string> Original_array;  //ԭʼ��������
	vector<string> Named_array;    //�����Զ�����������

	CString	strWorkPath;
	WCHAR wcsCurPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, wcsCurPath);
	strWorkPath = wcsCurPath;
	//strWorkPath += L"\\"; strWorkPath += strGuid;
	//CreateDirectory(strWorkPath, NULL);

	PCMD_INFO  pCmd_Info = pLibInfo->m_pBeginCmdInfo;

	for (int n = 0;n < pLibInfo->m_nCmdCount;n++) {
		Original_array.push_back((char*)pCmd_Info->m_szName);
		pCmd_Info++;
	}

	Named_array = Original_array;
	PLIB_DATA_TYPE_INFO pDataTypeInfo = pLibInfo->m_pDataType;
	for (int n = 0;n < pLibInfo->m_nDataTypeCount;n++) {
		LPINT CmdsIndex = pDataTypeInfo->m_pnCmdsIndex;
		for (int m = 0;m < pDataTypeInfo->m_nCmdCount;m++) {
			Named_array[*CmdsIndex] = (char*)pDataTypeInfo->m_szName;
			Named_array[*CmdsIndex].append(".");
			Named_array[*CmdsIndex].append(Original_array[*CmdsIndex]);
			CmdsIndex++;
		}
		pDataTypeInfo++;
	}


	PCMD_INFO  pCmd = pLibInfo->m_pBeginCmdInfo;
	LPINT pFunc = (LPINT)pLibInfo->m_pCmdsFunc;
	//������������������������������������������������������������������������
	//                     ����һ��Դ��

	/*string ECODE;
	ECODE.append(".�ֲ����� ����, ������\r\n");
	ECODE.append(".�ֲ����� �ı�, �ı���\r\n");
	ECODE.append(".�ֲ����� �ֽڼ�, �ֽڼ�\r\n");
	for (int i = 0;i < pLibInfo->m_nCmdCount;i++) {
		if (*pFunc == NULL) {           //���˿պ���,���� �������
			pCmd++;continue;
		}
		if (pCmd->m_wState == 32772) { //��Ч����,��һЩ�����������з����ó��Ľ��
			pCmd++;continue;
		}

		PARG_INFO arg = pCmd->m_pBeginArgInfo;
		string Efunc = (char*)pCmd->m_szName;
		Efunc = Efunc + '(';
		for (int n = 0;n < pCmd->m_nArgCount;n++) {
			switch (arg->m_dtDataType)
			{
			case 0x80000000:   //ͨ����
				Efunc.append("����,");
				break;
			case 0x80000005:   //�ֽڼ�
				Efunc.append("�ֽڼ�,");
				break;
			case 0x80000006:   //�ӳ���ָ��
				Efunc.append("&�ӳ���,");
				break;
			case 0x80000301:   //������
				Efunc.append("����,");
				break;
			case 0x80000004:   //�ı���
				Efunc.append("�ı�,");
				break;
			case 0x80000002:   //�߼���
				Efunc.append("��,");
				break;
			default:
				DebugMessage("%s-%X", pCmd->m_szName, arg->m_dtDataType);
				break;
			}
			arg++;
		}
		if (Efunc[Efunc.length() - 1] == ',') {
			Efunc.erase(Efunc.length() - 1);
		}
		
		Efunc = Efunc + ')';
		DebugMessage("%s", Efunc.c_str());
		ECODE.append(Efunc + "\r\n");
		pCmd++;
	}

	ECODE.append(".�ӳ��� �ӳ���\r\n");

	//���������
	HGLOBAL hClip;
	if (OpenClipboard()) {
		EmptyClipboard();
		hClip = GlobalAlloc(GMEM_MOVEABLE, ECODE.length() + 1);
		char *buff;
		buff = (char*)GlobalLock(hClip);	//�����ڴ�
		strcpy(buff, ECODE.c_str());
		GlobalUnlock(hClip);
		SetClipboardData(CF_TEXT, hClip);
		CloseClipboard();
	}
	return;*/
	
	//������������������������������������������������������������������������
	GenerateRelocationMap((unsigned char*)hLib, 0);  //����һ���ض�λ��ַ��

	GenerateIATMap((unsigned char*)hLib);   //����һ��IAT��

	DebugMessage("> ��ʼ����������===========================");
	//������������������������������������������������������������������������
	
	CString strTargetFile = strWorkPath;
	strTargetFile += L"\\";
	strTargetFile += A2W((char*)pLibInfo->m_szName);
	strTargetFile += strLibVer;
	strTargetFile += L".Esig";

	hFile = CreateFile(strTargetFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"����sig�ļ�ʧ��! ��ǰĿ¼����д ����û�й���ԱȨ��");
		return;
	}

	for (int i = 0; i < pLibInfo->m_nCmdCount; i++)
	{
		if (*pFunc == NULL){           //���˿պ���,���� �������
			pCmd++;pFunc++;continue;
		}
		if (pCmd->m_wState == 32772) { //��Ч����,��һЩ�����������з����ó��Ľ��
			pCmd++;pFunc++;continue;
		}

		string FuncName = Named_array[i];
		string FuncText;

		m_block.clear();
		Findprocend(*pFunc, FuncText);

		m_FuncOk[FuncName]= FuncText;

		DebugMessage("%s", FuncName.c_str());

		pCmd++; pFunc++;
	}

	DebugMessage("�������!������Ч��Ϊ:%d",m_FuncOk.size());	

	//������������������д���ļ���������������������������������
	CStringA strText;

	// д��Config��Ϣ
	WriteFile(hFile, "******Config******\r\n", 20, &dwWritten, NULL);

	strText.Format("Name=%s\r\n", pLibInfo->m_szName);
	WriteFile(hFile, strText.GetBuffer(), strText.GetLength(), &dwWritten, NULL);
	strText.Format("Description=%s\r\n", pLibInfo->m_szGuid);
	WriteFile(hFile, strText.GetBuffer(), strText.GetLength(), &dwWritten, NULL);

	WriteFile(hFile, "******Config_End******\r\n", 24, &dwWritten, NULL);

	// д��SubFunc��Ϣ
	WriteFile(hFile, "*****SubFunc*****\r\n", 19, &dwWritten, NULL);
	WriteFile(hFile, "*****SubFunc_End*****\r\n", 23, &dwWritten, NULL);

	// д��Func��Ϣ
	WriteFile(hFile, "***Func***\r\n", 12, &dwWritten, NULL);

	map<string, string>::iterator it;
	it = m_FuncOk.begin();
	while (it != m_FuncOk.end()) {
		WriteFile(hFile, (it->first + ":" + it->second + "\r\n").c_str(), it->first.length() + 3 + it->second.length(), &dwWritten, NULL);
		it++;
	}

	WriteFile(hFile, "***Func_End***\r\n", 16, &dwWritten, NULL);

	CloseHandle(hFile);
		
	if (hLib) {
		FreeLibrary(hLib);
	}
		
	DebugMessage("����Esig�ļ���ϣ�����");
	m_block.clear();
	m_FuncOk.clear();
	m_call.clear();
}



/*
	8B 44 24 0C 50 E8 ?? ?? ?? ?? 83 C4 04 8B C8
	68 ?? ?? ?? ?? B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? C7 40 04 00 00 00 00 B9 ?? ?? ?? ??
	8B 44 24 0C 50 E8 ?? ?? ?? ?? 8B C8 83 C4 04 83 C1 5C
*/
	
void CCodMakeDlg::OnDropFiles(HDROP hDropInfo)
{
	WCHAR wcStr[MAX_PATH] = { 0 };
	CString	strFile;
	int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);//ȡ�ñ��϶��ļ�����Ŀ  
	DragQueryFile(hDropInfo, 0, wcStr, MAX_PATH);//�����ҷ�ĵ�i���ļ����ļ���
	DragFinish(hDropInfo);  //�ϷŽ�����,�ͷ��ڴ�  
	MakeCode(wcStr);
	CDialog::OnDropFiles(hDropInfo);
}
