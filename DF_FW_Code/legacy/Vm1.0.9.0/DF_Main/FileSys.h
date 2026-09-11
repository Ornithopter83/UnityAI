//
// FileSys.h
//

#ifndef _FILESYS_h
#define _FILESYS_h

#include "arduino.h"

#include "SPIFFS.h"				// File System

//----------------------------------------------------------------------
#define NO_FILE_MANUF_INFO 		"999999,999999,9999-9999"
#define NO_FILE_INSTALL_INFO	"9999-9999"

//-----------------------------------------------
#define NO_FILE_FS_INFO		"-11"	// 파일없음
#define NUM_NO_FILE_FS_INFO		-11
#define NO_VAL_FS_INFO		"-12"	// 파일내 데이타 없음
#define NUM_NO_VAL_FS_INFO		-12
#define PARA_NG_FS_INFO		"-13"	// CMD_PARA NG
#define NUM_PARA_NG_FS_INFO		-13
#define OPEN_FAIL_FS_INFO		"-14"	// Open Fail
#define NUM_OPEN_FAIL_FS_INFO	-14


//#define now_message_str String 		// TEST String ( Main <-> ROD)

class FileSys
{
  public:
    FileSys();
    void init();	// NG시 Reset

    String getInfo(short fsKind);
    short saveInfo(short fsKind, String _strManufInfo);
	short deleteInfo(short fsKind);
	// short checkValue(short fsKind);
	bool isNumeric(String str);
	
  private:
    //void test();
	//static char *ptrFileName;
};


#endif
