/*读取fat12*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  BytsPerSec;	
int  SecPerClus;	
int  RsvdSecCnt;	
int  NumFATs;	
int  RootEntCnt;	
int  FATSz;	


#pragma pack (1) /*指定按1字节对齐*/

//偏移11个字节
struct BPB {
	short  BPB_BytsPerSec;	
	char   BPB_SecPerClus;	
	short  BPB_RsvdSecCnt;	//记录占用的扇区数
    char   BPB_NumFATs;	
	short  BPB_RootEntCnt;	
	short  BPB_TotSec16;
	char   BPB_Media;
	short  BPB_FATSz16;	//FAT扇区数
	short  BPB_SecPerTrk;
	short  BPB_NumHeads;
	int  BPB_HiddSec;
	int  BPB_TotSec32;	
};

//根目录条目
struct RootEntry {
	char DIR_Name[11];
	char   DIR_Attr;		//文件属性
	char reserved[10];
	short  DIR_WrtTime;
	short  DIR_WrtDate;
	short  DIR_FstClus;	//开始簇号
	int  DIR_FileSize;
};
//根目录条目结束，32字节

//用来储存文件
struct Book {
	char name[100];
	short clus;
};


#pragma pack () /*取消指定对齐，恢复缺省对齐*/

struct Book list[100];
int leng = 0;

void fillBPB(FILE * fat12 , struct BPB* bpb_ptr);	
void printFiles(FILE * fat12 , struct RootEntry* rootEntry_ptr);	
void printNext(FILE * fat12 , char * directory,int startClus);	
int  getFAT(FILE * fat12 , int num);	
void printBPB(struct BPB* p);
void myRead(int clus);
void printName(char* msg);
void myprint(char* msg,int no,int leng,int color);//1代表原色，0代表蓝色


int main() {
	FILE* fat12;
	fat12 = fopen("a.img","rb");	
    
	struct BPB bpb;
	struct BPB* bpb_ptr = &bpb;
	//载入BPB

	fillBPB(fat12,bpb_ptr);
	
	//初始化各个全局变量
	BytsPerSec = bpb_ptr->BPB_BytsPerSec;
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
	NumFATs = bpb_ptr->BPB_NumFATs;
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	
	FATSz = bpb_ptr->BPB_FATSz16;
	
    
	struct RootEntry rootEntry;
	struct RootEntry* rootEntry_ptr = &rootEntry;
    //printBPB(bpb_ptr);
	//打印文件名
	printFiles(fat12,rootEntry_ptr);
    
	fclose(fat12);
	myprint("Input 'ENDIT' to end program\n",29,29,1);
	for(;;){
		int isRight = 0;
		int isTxt = 0;
		int m;
		char na[100];
		scanf("%s",na);
		fflush(stdin);
		int length = strlen(na);
		if(strcmp(na,"ENDIT")==0){
			break;
		}
		//从文件读
		for(m=0;m<leng;m++){
			if(strcmp(list[m].name,na)==0){
				myRead(list[m].clus);
				myprint("\n",1,1,1);
				isRight = 1;
				isTxt = 1;
			}
		}
		if(isTxt==1){
			continue;
		}
		//从文件夹读文件
		for(m=0;m<leng;m++){
			char tem[length+1];
			strncpy(tem,list[m].name,length);
			tem[length] = '\0';
			if(strcmp(tem,na)==0){
				printName(list[m].name);
				isRight = 1;
			}
		}
		//输入不合法
		if(isRight == 0){
			myprint("Unknown file\n",13,13,1);
		}
	}
}

int  getFAT(FILE * fat12 , int num) {  
    //FAT1的偏移字节  
    int fatBase = RsvdSecCnt * BytsPerSec;  
    int fatPos = fatBase + num*3/2;  
    int type = 0;  
    if (num % 2 == 0) {  
        type = 0;  
    } else {  
        type = 1;  
    }  
    unsigned short bytes;  
    unsigned short* bytes_ptr = &bytes;  
    int check;  
    check = fseek(fat12,fatPos,SEEK_SET);  
    check = fread(bytes_ptr,1,2,fat12);  
  
    if (type == 0) {  
        bytes = bytes<<4;
        return bytes>>4;  
    } else {  
        return bytes>>4;  
    }  
 
}

void printName(char* msg){
	int len1 = strlen(msg);
	int i = 0;
	int k = 0;
	for(i=0;i<len1;i++){
       if(msg[i] == '/'){
       	   k = i;
       }       
	}
	if(k==0){
		myprint(msg,strlen(msg),strlen(msg),1);
		return;
	}
	char dir[k+2];
	char filename[len1-k];
	for(i = 0;i<=k;i++){
		dir[i] = msg[i];
	}
	dir[k+1] = '\0';
	for(i = 0;i<len1-k-1;i++){
		filename[i] = msg[k+i+1];
	}
	filename[len1-k-1] = '\0';
	myprint(dir,strlen(dir),strlen(dir),0);
	myprint(filename,strlen(filename),strlen(filename),1);
	myprint("\n",1,1,1);
}

void myRead(int clus) {
	if(clus==-1){
		myprint("It's a directory",16,16,1);
		return;
	}
	FILE* fat12;
	fat12 = fopen("a.img","rb");	//打开FAT12的映像文件
    
	struct BPB bpb;
	struct BPB* bpb_ptr = &bpb;
	//载入BPB

	fillBPB(fat12,bpb_ptr);
	
	//初始化各个全局变量
	BytsPerSec = bpb_ptr->BPB_BytsPerSec;
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
	NumFATs = bpb_ptr->BPB_NumFATs;
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	
	FATSz = bpb_ptr->BPB_FATSz16;
	int currentClus = clus;
	int dataBase = BytsPerSec * ( RsvdSecCnt + FATSz*NumFATs + (RootEntCnt*32 + BytsPerSec - 1)/BytsPerSec );
	//第一个簇从2开始的
	int value = getFAT(fat12,currentClus);
        
	char* str = (char* )malloc(SecPerClus*BytsPerSec);	
	char* content = str;
	int startByte = dataBase + (currentClus - 2)*SecPerClus*BytsPerSec;
	int check;
	check = fseek(fat12,startByte,SEEK_SET);
       
	check = fread(content,1,SecPerClus*BytsPerSec,fat12);
    int i;
    for(i=0;i<check;i++){
    	if(content[i]!=0){
    		char temp[2];
    		temp[0] = content[i];
    		temp[1] = '\0';
    		myprint(temp,1,1,1);
    	}
    }
    //判断是不是有下个簇
	free(str);
	fclose(fat12);
	if(value<0xF88){
    	myRead(value);
    }

}
/*
void printBPB(struct BPB* p) {
	printf("fat:");
	printf("%d\n",p->BPB_NumFATs);
	printf("sec:");
	printf("%d\n",p->BPB_BytsPerSec);
	printf("clus:");
	printf("%d\n",p->BPB_SecPerClus);
	printf("all:");
	printf("%d\n",p->BPB_FATSz16);
}*/

void fillBPB(FILE* fat12 , struct BPB* bpb_ptr) {
	int check;
    
	//BPB从偏移11个字节处开始
	check = fseek(fat12,11,SEEK_SET);
	check = fread(bpb_ptr,1,25,fat12);
}



void printFiles(FILE * fat12 , struct RootEntry* rootEntry_ptr) {
	int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;	//根目录
	int check;
	char realName[12];	
	int i;
	for (i=0;i<RootEntCnt;i++) {
        
		check = fseek(fat12,base,SEEK_SET);
		check = fread(rootEntry_ptr,1,32,fat12);
		base += 32;
        
		if (rootEntry_ptr->DIR_Name[0] == '\0') continue;	//空条目不输出
        
		//过滤非目标文件
		int j;
		int boolean = 0;
		for (j=0;j<11;j++) {
			if (!(((rootEntry_ptr->DIR_Name[j] >= 48)&&(rootEntry_ptr->DIR_Name[j] <= 57)) ||
                  ((rootEntry_ptr->DIR_Name[j] >= 65)&&(rootEntry_ptr->DIR_Name[j] <= 90)) ||
                  ((rootEntry_ptr->DIR_Name[j] >= 97)&&(rootEntry_ptr->DIR_Name[j] <= 122)) ||
                  (rootEntry_ptr->DIR_Name[j] == ' '))) {
				boolean = 1;	
				break;
			}
		}
		if (boolean == 1) continue;	//非目标文件不输出
        
		int k;
		if ((rootEntry_ptr->DIR_Attr&0x10) == 0 ) {
			//文件
			int tempLong = -1;
			for (k=0;k<11;k++) {
				if (rootEntry_ptr->DIR_Name[k] != ' ') {
					tempLong++;
					realName[tempLong] = rootEntry_ptr->DIR_Name[k];
				} else {
					tempLong++;
					realName[tempLong] = '.';
					while (rootEntry_ptr->DIR_Name[k] == ' ') k++;
					k--;
				}
			}
			tempLong++;
			realName[tempLong] = '\0';	
            //输出文件
            list[leng].clus = rootEntry_ptr->DIR_FstClus;
            strcpy(list[leng].name,realName);
            leng++;
			myprint(realName,strlen(realName),strlen(realName),1);
			myprint("\n",1,1,1);
		} else {
			//目录
			int tempLong = -1;
			for (k=0;k<11;k++) {
				if (rootEntry_ptr->DIR_Name[k] != ' ') {
					tempLong++;
					realName[tempLong] = rootEntry_ptr->DIR_Name[k];
				} else {
					tempLong++;
					realName[tempLong] = '\0';
					break;
				}
			}	
            
			//输出目录及子文件
			
			printNext(fat12,realName,rootEntry_ptr->DIR_FstClus);
		}
	}
}



void printNext(FILE * fat12 , char * directory , int startClus) {
	//数据区的第一个簇（即2号簇）的偏移字节
	int dataBase = BytsPerSec * ( RsvdSecCnt + FATSz*NumFATs + (RootEntCnt*32 + BytsPerSec - 1)/BytsPerSec );
	char fullName[100];	
	int strLength = strlen(directory);
	strcpy(fullName,directory);
	fullName[strLength] = '/';
	strLength++;
	fullName[strLength] = '\0';
	char* fileName = &fullName[strLength];
    
	int currentClus = startClus;
	int value = 0;
	int ifOnlyDirectory = 0;
    while (value < 0xFF8) {
		value = getFAT(fat12,currentClus);
        
		char* str = (char* )malloc(SecPerClus*BytsPerSec);	
		char* content = str;
		
		int startByte = dataBase + (currentClus - 2)*SecPerClus*BytsPerSec;
		//printf("byte:");
		//printf("%d\n",startByte);
		int check;
		check = fseek(fat12,startByte,SEEK_SET);
        
		check = fread(content,1,SecPerClus*BytsPerSec,fat12);
		//与根目录相同
		int count = SecPerClus*BytsPerSec;	
		int loop = 0;
		while (loop < count) {
			int i;
			char tempName[12];	
			if (content[loop] == '\0') {
				loop += 32;
				continue;
			}	
			int j;
			int boolean = 0;
			for (j=loop;j<loop+11;j++) {
				if (!(((content[j] >= 48)&&(content[j] <= 57)) ||
                      ((content[j] >= 65)&&(content[j] <= 90)) ||
                      ((content[j] >= 97)&&(content[j] <= 122)) ||
                      (content[j] == ' '))) {
                    boolean = 1;	
                    break;
				}
			}
			if (boolean == 1) {
				loop += 32;
				continue;
			}
			int k;
			char kind = content[loop+11];
			if ((kind&0x10) == 0 ) {
				//是文件
				int tempLong = -1;
				for (k=0;k<11;k++) {
					if (content[loop+k] != ' ') {
						tempLong++;
						tempName[tempLong] = content[loop+k];
					} else {
						tempLong++;
						tempName[tempLong] = '.';
						while (content[loop+k] == ' ') k++;
						k--;
					}
				}
				tempLong++;
				tempName[tempLong] = '\0';	//放到tempName里
                strcpy(fileName,tempName);
                strcpy(list[leng].name,fullName);
                short t1 = (short)content[loop+26];
				short t2 = (short)content[loop+27]<<8;
				short end= t1 + t2;
				list[leng].clus = end;
				leng++;
                printName(fullName);
				ifOnlyDirectory = 1;
				loop += 32;
			}
			else{
				// 文件夹
				int tempLong = -1;
				for (k=0;k<11;k++) {
					if (content[k+loop] != ' ') {
						tempLong++;
						tempName[tempLong] = content[k+loop];
					} else {
						tempLong++;
						tempName[tempLong] = '\0';
						break;
					}
				}
				strcpy(fileName,tempName);
				ifOnlyDirectory=1;
				//递归显示
				short t1 = (short)content[loop+26];
				short t2 = (short)content[loop+27]<<8;
				short end= t1 + t2;
				//printf("child:");
				//int i;
				/*for(i=0;i<30;i++){
					short tt = (short)content[loop+i];
					printf("%d\n",tt);
				}*/
				printNext(fat12,fullName,end);

				loop+=32;
			}
		}
        
		free(str);
        
		currentClus = value;
	};
    
    if (ifOnlyDirectory == 0){
        myprint(fullName,strlen(fullName),strlen(fullName),0);
        myprint("\n",1,1,1);
        strcpy(list[leng].name,fullName);
        list[leng].clus = -1;
        leng++;
        //空目录的情况下，输出目录
    }
}