#include <efi.h>
#include <efilib.h>
#include "mini-printf/mini-printf.h"
#include "util.h"
#include "efitable.h"
#include "ubasic/ubasic.h"

EFI_SYSTEM_TABLE *gST;

EFI_SYSTEM_TABLE **ub_get_eST(){
	return &gST;
}
void *ub_malloc(int bytes){
	void *mem;
	gST->BootServices->AllocatePool(EfiLoaderData,bytes,&mem);
	return mem;
}

void ub_free(void *ptr){
	gST->BootServices->FreePool(ptr);
}

void ub_space(char *str){
	int i;
	int len = ub_strlen(str);
	for(i=0;i<len;i++){
		if(str[i] == '\r' && str[i+1] == '\n'){
			str[i] = '\x20';
		}
	}
}
void ub_unspace(char *str){
	int i;
	int len = ub_strlen(str);
	for(i=0;i<len;i++){
		if(str[i] == '\x20' && str[i+1] == '\n'){
			str[i] = '\r';
		}
	}
}
EFI_STATUS efi_main(EFI_HANDLE eIH, EFI_SYSTEM_TABLE *eST){
	CHAR16 buf[512];
	CHAR16 *bufptr = &buf[0];
	char cbuf[512];
	char linebuf[1024];
	int i = 0;
	gST = eST;
	/*Initialize buffer memory*/
	for(i=0;i<50;i++){
		buf[i] = 0;
	}
	eST->ConOut->OutputString(eST->ConOut,L"UEFI BASIC v0.1\r\n");
	eST->ConOut->OutputString(eST->ConOut,L"Ok\r\n");
	ub_memset(linebuf,0,1024);
	while(1){
		ub_readline(eST,&bufptr);
		ub_u2c(cbuf,buf,500);
		if(ub_isdigit(cbuf[0])){
			int i, digit, len, replace_flag;
			char digit_str[8];
			replace_flag = 0;
			len = ub_strlen(linebuf);
			digit = ub_atoi(cbuf);
			mini_snprintf(digit_str,5,"%d",digit);
			if(len > 2 && ub_isdigit(linebuf[0])){
				int cur_digit = ub_atoi(linebuf);
				if(cur_digit == digit){
					replace_flag = 1;
					int orig_line_len;
					char backup_linebuf[1024];
					for(orig_line_len=0;linebuf[orig_line_len] != '\n';orig_line_len++){
					}
					ub_memcpy(backup_linebuf,&linebuf[orig_line_len+1],1024);
					len = 0; /* skip the for loop to not look for a match again */
					/* use snprintf() to grow the string */
					mini_snprintf(linebuf,1024,"%s%s",cbuf,backup_linebuf);
				}
			}
			for(i=0;i<len;i++){
				if(linebuf[i] == '\n' && ub_isdigit(linebuf[i+1])){
					int cur_digit = ub_atoi(&(linebuf[i+1]));
					if(cur_digit == digit){
						char truncated_linebuf[1024];
						char backup_linebuf[1024];
						int orig_line_start, end_line_start;
						int retval;
						replace_flag = 1;
						orig_line_start = i+1;
						retval = mini_snprintf(truncated_linebuf,orig_line_start+1,"%s",linebuf);
						for(end_line_start=orig_line_start;linebuf[end_line_start] != '\n';end_line_start++){
						}
						for(retval=0;retval<30;retval++)
						ub_memcpy(backup_linebuf,linebuf,1024);
						ub_memset(linebuf,0,1024); /*TODO: remove this*/
						mini_snprintf(linebuf,1024,"%s%s%s",truncated_linebuf,cbuf,&backup_linebuf[end_line_start+1]);
					}
				}
			}
			if(!replace_flag){
				mini_snprintf(linebuf,1000,"%s%s",linebuf,cbuf);
			}
		}
		else if(ub_strncmp(cbuf,"run\r\n",6) == 0 || ub_strncmp(cbuf,"RUN\r\n",6) == 0){
			ub_space(linebuf);
			ubasic_init(linebuf);
			do{
				ubasic_run();
			}
			while(!ubasic_finished());
			ub_unspace(linebuf);
			eST->ConOut->OutputString(eST->ConOut,L"Ok\r\n");
		}
		else if(ub_strncmp(cbuf,"list\r\n",7) == 0 || ub_strncmp(cbuf,"LIST\r\n",7) == 0){
			ub_printf("%s",linebuf);
			eST->ConOut->OutputString(eST->ConOut,L"Ok\r\n");
		}
		else if(ub_strncmp(cbuf,"new\r\n",6) == 0 || ub_strncmp(cbuf,"NEW\r\n",6) == 0){
			ub_memset(linebuf,0,1024);
			eST->ConOut->OutputString(eST->ConOut,L"Ok\r\n");
		}
		else{
			eST->ConOut->OutputString(eST->ConOut,L"Syntax error\r\n");
		}
		ub_memset(buf,0,512);
	}
	return EFI_SUCCESS;
}
