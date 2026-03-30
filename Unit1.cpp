//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TForm1 *Form1;


TLFSR::TLFSR()
{
	regSize = 27;
    mask = (1 << 26) | (1 << 7) | (1 << 6) | (1 << 0);
    state = 0;
}

//---------------------------------------------------------------------------
void TLFSR::setState(const String &initState)
{
    state = 0;
    int len = initState.Length();
    if (len > regSize) len = regSize;

    for (int i = 0; i < len; i++)
    {
        if (initState[i + 1] == '1')
		{
            state |= (1 << (len - 1 - i));
        }
    }
}

//---------------------------------------------------------------------------
bool TLFSR::nextBit()
{
	bool outputBit = (state >> (regSize - 1)) & 1;

    unsigned int feedback = 0;
    for (int i = 0; i < regSize; i++)
    {
        if (mask & (1 << i))
        {
            feedback ^= ((state >> i) & 1);
        }
    }

	state = (state << 1) & ((1 << regSize) - 1);
	state |= feedback;

    return outputBit;
}

//---------------------------------------------------------------------------
String TLFSR::generateKey(int length)
{
    String result = "";
    for (int i = 0; i < length; i++)
    {
        result += nextBit() ? '1' : '0';
    }
    return result;
}

//---------------------------------------------------------------------------
String TLFSR::getStateBinary() const
{
    String result = "";
    for (int i = regSize - 1; i >= 0; i--)
    {
        result += ((state >> i) & 1) ? '1' : '0';
    }
    return result;
}

__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    originalData = new TMemoryStream();
    encryptedData = new TMemoryStream();
    currentKey = "";

    edtInitState->MaxLength = 27;
    edtInitState->Text = "";
	edtKeyLength->Text = "0";
	edtKeyLength->ReadOnly = true;

	GenerateKey->Enabled = false;
    encrypt->Enabled = false;
	decrypt->Enabled = false;
    save->Enabled = false;

	UpdateLabelsInfo();
}

//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1()
{
    delete originalData;
    delete encryptedData;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateLabelsInfo()
{
    String currentState = edtInitState->Text;
    int actualLength = 0;
    for (int i = 1; i <= currentState.Length(); i++)
    {
        if (currentState[i] == '0' || currentState[i] == '1')
            actualLength++;
    }
    Label1->Caption = "Введено бит: " + IntToStr(actualLength) + " из " +
                      IntToStr(lfsr.getRegSize());

    if (originalData && originalData->Size > 0)
    {
		int requiredBits = originalData->Size * 8;
        edtKeyLength->Text = IntToStr(requiredBits);
    }
    else
	{
		edtKeyLength->Text = "0";
	}

    Label4->Caption = "Полином: x^27 + x^8 + x^7 + x + 1";

    if (actualLength == 27 && originalData && originalData->Size > 0)
    {
        GenerateKey->Enabled = true;
    }
    else
    {
        GenerateKey->Enabled = false;
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateKeyLengthInfo()
{
    if (originalData && originalData->Size > 0)
    {
		int requiredBits = originalData->Size * 8;
        edtKeyLength->Text = IntToStr(requiredBits);

        String currentState = edtInitState->Text;
        int actualLength = 0;
        for (int i = 1; i <= currentState.Length(); i++)
        {
            if (currentState[i] == '0' || currentState[i] == '1')
                actualLength++;
        }

        if (actualLength == 27)
        {
            GenerateKey->Enabled = true;
        }
        else
        {
            GenerateKey->Enabled = false;
        }
    }
    else
	{
        edtKeyLength->Text = "0";
        GenerateKey->Enabled = false;
        encrypt->Enabled = false;
        decrypt->Enabled = false;
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::AutoGenerateKey()
{
	if (!originalData || originalData->Size == 0)
    {
        Application->MessageBox(L"Сначала загрузите файл!", L"Ошибка", MB_ICONERROR);
        return;
    }

    String initState = edtInitState->Text;

    int actualLength = 0;
    for (int i = 1; i <= initState.Length(); i++)
    {
        if (initState[i] == '0' || initState[i] == '1')
            actualLength++;
    }

    if (actualLength != 27)
	{
        Application->MessageBox(L"Введите все 27 бит начального состояния регистра!",
            L"Ошибка", MB_ICONERROR);
        return;
	}

    for (int i = 1; i <= initState.Length(); i++)
    {
        if (initState[i] != '0' && initState[i] != '1')
        {
			Application->MessageBox(L"Состояние регистра может содержать только 0 и 1!",
				L"Ошибка ввода", MB_ICONERROR);
			return;
        }
    }

    String fullState = initState;
    while (fullState.Length() < 27)
    {
        fullState = "0" + fullState;
    }

    lfsr.setState(fullState);

    int keyLength = originalData->Size * 8;
	if (keyLength <= 0)
	{
		Application->MessageBox(L"Файл пуст!",
				L"Ошибка ввода", MB_ICONERROR);
		return;
    }

    currentKey = lfsr.generateKey(keyLength);

    memoKey->Clear();
    if (currentKey.Length() > 200)
    {
        memoKey->Lines->Add(currentKey.SubString(1, 200) + "...");
        memoKey->Lines->Add(L"(Показано первых 200 бит из " + IntToStr(currentKey.Length()) + L")");
    }
    else
    {
        memoKey->Lines->Add(currentKey);
	}

    encrypt->Enabled = true;
	decrypt->Enabled = true;
	save->Enabled = false;

    UpdateLabelsInfo();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateBinaryView(TMemo *memo, TMemoryStream *data)
{
    memo->Clear();
    if (!data || data->Size == 0)
    {
        memo->Lines->Add("[Нет данных]");
        return;
    }

    data->Position = 0;
    int size = data->Size;
    unsigned char *buffer = new unsigned char[size];
	data->Read(buffer, size);

    int maxBytes = (size > 500) ? 500 : size;

    for (int i = 0; i < maxBytes; i++)
    {
        String binary = "";
        for (int bit = 7; bit >= 0; bit--)
        {
            binary += ((buffer[i] >> bit) & 1) ? '1' : '0';
        }
        memo->Lines->Add(binary);
    }

    if (size > 500)
    {
        memo->Lines->Add("...");
        memo->Lines->Add("(Показано первых 500 байт из " + IntToStr(size) + ")");
    }

    delete[] buffer;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ProcessFile(bool encrypt)
{
    if (originalData->Size == 0)
    {
        Application->MessageBox(L"Сначала загрузите файл!", L"Ошибка", MB_ICONERROR);
        return;
    }

    if (currentKey.IsEmpty())
    {
        Application->MessageBox(L"Сначала сгенерируйте ключ!", L"Ошибка", MB_ICONERROR);
        return;
    }

    int requiredLength = originalData->Size * 8;
    if (currentKey.Length() != requiredLength)
	{
		AutoGenerateKey();
        if (currentKey.Length() != requiredLength)
        {
            Application->MessageBox(L"Ошибка генерации ключа!", L"Ошибка", MB_ICONERROR);
            return;
        }
    }

    encryptedData->Clear();
    originalData->Position = 0;
    int dataSize = originalData->Size;
    unsigned char *buffer = new unsigned char[dataSize];
    originalData->Read(buffer, dataSize);

    for (int i = 0; i < dataSize; i++)
    {
        unsigned char result = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            int keyIndex = i * 8 + bit;
            unsigned char keyBit = (currentKey[keyIndex + 1] == '1') ? 1 : 0;
            unsigned char dataBit = (buffer[i] >> (7 - bit)) & 1;
            unsigned char outBit = dataBit ^ keyBit;
            result |= (outBit << (7 - bit));
        }
        buffer[i] = result;
    }

    encryptedData->Write(buffer, dataSize);
    delete[] buffer;

	String operation = encrypt ? L"Зашифрован" : L"Расшифрован";

    UpdateBinaryView(memoEncryptedBinary, encryptedData);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::LoadFileClick(TObject *Sender)
{
    if (OpenDialog1->Execute())
    {
		originalData->Clear();
		originalData->LoadFromFile(OpenDialog1->FileName);

        UpdateKeyLengthInfo();

        UpdateBinaryView(memoOriginalBinary, originalData);

        currentKey = "";
        memoKey->Clear();
        encrypt->Enabled = false;
		decrypt->Enabled = false;
		save->Enabled = false;

        UpdateLabelsInfo();

        String currentState = edtInitState->Text;
        int actualLength = 0;
        for (int i = 1; i <= currentState.Length(); i++)
        {
            if (currentState[i] == '0' || currentState[i] == '1')
                actualLength++;
        }

    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::encryptClick(TObject *Sender)
{
	ProcessFile(true);
	save->Enabled = true;
    Application->MessageBox(L"Шифрование выполнено!", L"Результат", MB_ICONINFORMATION);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::decryptClick(TObject *Sender)
{
	ProcessFile(false);
	save->Enabled = true;
    Application->MessageBox(L"Расшифрование выполнено!", L"Результат", MB_ICONINFORMATION);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::saveClick(TObject *Sender)
{
    if (encryptedData->Size == 0)
    {
        Application->MessageBox(L"Нет данных для сохранения! Сначала выполните шифрование/дешифрование.",
            L"Ошибка", MB_ICONERROR);
        return;
    }

	if (SaveDialog1->Execute())
	{
		encryptedData->SaveToFile(SaveDialog1->FileName);
        Application->MessageBox(L"Файл сохранен!", L"Результат", MB_ICONINFORMATION);
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm1::GenerateKeyClick(TObject *Sender)
{
    AutoGenerateKey();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ViewOriginalClick(TObject *Sender)
{
    if (originalData->Size == 0)
    {
        Application->MessageBox(L"Нет исходного файла!", L"Ошибка", MB_ICONERROR);
        return;
    }
    UpdateBinaryView(memoOriginalBinary, originalData);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::ViewEncryptedClick(TObject *Sender)
{
    if (encryptedData->Size == 0)
    {
        Application->MessageBox(L"Нет зашифрованных данных!", L"Ошибка", MB_ICONERROR);
        return;
    }
    UpdateBinaryView(memoEncryptedBinary, encryptedData);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::edtInitStateChange(TObject *Sender)
{
	UpdateLabelsInfo();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::edtInitStateKeyPress(TObject *Sender, System::WideChar &Key)
{
	if (Key != '0' && Key != '1' && Key != VK_BACK && Key != VK_DELETE &&
        Key != VK_LEFT && Key != VK_RIGHT && Key != VK_HOME && Key != VK_END)
	{
        Application->MessageBox(L"Можно вводить только 0 и 1!", L"Ошибка ввода", MB_ICONWARNING);
    }
}


