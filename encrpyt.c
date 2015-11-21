#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#define BUFFSIZE 1024
#define PUBLICKEY "test_pub.key"
#define OPENSSLKEY "test.key"
char* encrypt(char*str,char*path);
char* decrypt(char*str,char*path);

int main(void)
{
	FILE*fp;
	char * source="are you sb?";
	char* ptr_en,*ptr_de;
	printf("source is :%s\n",source);
	ptr_en=encrypt(source,PUBLICKEY);
//	printf("after ebcrypt:%s\n",ptr_en);
	ptr_de=decrypt(ptr_en,OPENSSLKEY);
	printf("after decrypt:%s\n",ptr_de);
	if(ptr_de!=NULL)
		free(ptr_de);
	if(ptr_en!=NULL)
		free(ptr_en);
	return 0;
}

char*encrypt(char*str,char*path)
{
	char*p_en;
	RSA * p_rsa;
	FILE*file;
	int flen,rsa_len;
	if((file=fopen(path,"r"))==NULL)
	{
		perror("open key file error");
		return NULL;
	}
	if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL)
	{
		ERR_print_errors_fp(stdout);
		return NULL;
	}
	flen=strlen(str);
	rsa_len=RSA_size(p_rsa);
	p_en=(unsigned char*)malloc(rsa_len+1);
	memset(p_en,0,rsa_len+1);
	if(RSA_public_encrypt(rsa_len,(unsigned char*)str,(unsigned char*)p_en,p_rsa,RSA_NO_PADDING)<0)
	{
		return NULL;
	}
	RSA_free(p_rsa);
	fclose(file);
	return p_en;
}

char* decrypt(char*str,char*path)
{
	char*p_de;
	RSA *p_rsa;
	FILE*file;
	int rsa_len;
	if((file=fopen(path,"r"))==NULL)
	{
		perror("open key file error");
		return NULL;
	}
	if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL)
	{
		ERR_print_errors_fp(stdout);
		return NULL;
	}
	rsa_len=RSA_size(p_rsa);
	p_de=(unsigned char *)malloc(rsa_len+1);
	memset(p_de,0,rsa_len+1);
	if(RSA_private_decrypt(rsa_len,(unsigned char*)str,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0)
	{
		return NULL;
	}
	RSA_free(p_rsa);
	fclose(file);
	return p_de;
}
