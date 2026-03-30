#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------

class TLFSR
{
private:
    unsigned int state;      // Текущее состояние регистра (27 бит)
    unsigned int mask;       // Маска для обратной связи
    int regSize;             // Размер регистра

public:
    TLFSR();
    void setState(const String &initState);
    unsigned int getState() const { return state; }
    bool nextBit();          // Генерация следующего бита ключа
    String generateKey(int length);  // Генерация ключа заданной длины
    String getStateBinary() const;   // Получение состояния в двоичном виде
    int getRegSize() const { return regSize; }
};

class TForm1 : public TForm
{
__published:
	TButton *LoadFile;
	TEdit *edtInitState;
	TButton *encrypt;
	TButton *decrypt;
	TButton *save;
	TLabel *Label1;
	TEdit *edtKeyLength;
	TButton *GenerateKey;
	TMemo *memoKey;
	TMemo *memoOriginalBinary;
	TLabel *Label4;
	TMemo *memoEncryptedBinary;
	TOpenDialog *OpenDialog1;
	TSaveDialog *SaveDialog1;
	void __fastcall LoadFileClick(TObject *Sender);
	void __fastcall encryptClick(TObject *Sender);
	void __fastcall decryptClick(TObject *Sender);
	void __fastcall saveClick(TObject *Sender);
	void __fastcall GenerateKeyClick(TObject *Sender);
	void __fastcall ViewOriginalClick(TObject *Sender);
	void __fastcall ViewEncryptedClick(TObject *Sender);
	void __fastcall edtInitStateChange(TObject *Sender);
	void __fastcall edtInitStateKeyPress(TObject *Sender, System::WideChar &Key);

           // Добавлено

private:
    TMemoryStream *originalData;      // Исходные данные
    TMemoryStream *encryptedData;     // Зашифрованные данные
    String currentKey;                // Сгенерированный ключ
    TLFSR lfsr;                       // Генератор LFSR

    void __fastcall UpdateBinaryView(TMemo *memo, TMemoryStream *data);
    void __fastcall ProcessFile(bool encrypt);
    void __fastcall AutoGenerateKey();
    void __fastcall UpdateLabelsInfo();  // Обновление информации в Label
    void __fastcall UpdateKeyLengthInfo(); // Обновление информации о длине ключа

public:
    __fastcall TForm1(TComponent* Owner);
    __fastcall ~TForm1();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
