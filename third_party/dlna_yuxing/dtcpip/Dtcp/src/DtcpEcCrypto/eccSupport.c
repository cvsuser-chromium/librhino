//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#include <string.h>
#include "eccSupport.h"
#include "icl.h"
#include "rsa.h"
#include "rsakg.h"
#include "Rng.h"

// pointsize = 320bits, 40 bytes = 10 * sizeof (W32)
#define POINTSIZE_BYTES 40
#define POINTSIZE_WORDS 10

char *stringify(W8 *x, char *buff, int bufflen)
{
	int i;
	for (i=0;i<bufflen;i++) 
	{
		sprintf(buff+2*i,"%02x",x[i]);
	}
	return buff;
}

int byteLength160(W8 *x, RSAData *a)
{
	// input is assumed BIG ENDIAN
	int i;
	int byteLength=20;
	for (i=0;i<20;i++,byteLength--) {
		if (0==x[i] && i!=19) 
		{
			continue;
		}
		else 
		{
			a->length=byteLength;
			break;
			
		}
	}
	return byteLength;
}

int w32WordLength160(W8 *x, RSAData *a)
{
	// input is assumed BIG ENDIAN
	int i;
	int wordLength=5;
	for (i=0;i<5;i++,wordLength--) {
		if (0==((W32 *)x)[i] && i!=4) 
		{
			continue;
		}
		else 
		{
			a->length=wordLength;
			break;
		}
	}
	return wordLength;
}

void *bswap160(W8 *a,W8 *dest)
{
	int i;
	if (a==dest)
	{
		W8 temp;
		for (i=0;i<10;i++) {
			temp= dest[19-i];
			dest[19-i]=a[i];
			a[i]= temp;
		}
	}
	else
	{
		for (i=0;i<20;i++) {
			dest[19-i]=a[i];
		}
	}
	return dest;
}

void f_msb_bits_in_r (W8 *f, const W8 *r)
{
	//calc bits in r
	//count unused bits
	int i,bits,offset_bytes;
	W8 mask =0x80;
	for(i=0,bits=160;i<20;i++)
	{
		if (r[i]) {
			//ok which bit in the byte
			while (0==(r[i] & mask))
			{
					mask>>=1;
					bits--;
			}
			break;
		}
		else {
			bits-=8;
		}
	}

	offset_bytes = (160-bits)/8;
	//rightshift 
	bits =8-(bits%8);
	i=20;
	while(i--)
	{
		f[i]= ((i>=offset_bytes)? (f[i-offset_bytes] >> bits) : 0)
			  | ((i-1>=offset_bytes)? (f[i-1-offset_bytes] << (8-bits)) : 0);
	}


}


/* Calculate digest= SHA1(pBuffer) */
void
SHA1(
	 W8 *digest,           /* OUT: 160 bit digest */ 
	 W8 *pBuffer,           /* IN: Buffer pointer */
	 W32 bufferLengthBytes  /* IN: Buffer length */
	 )
{
	ICLSHAState SHAState;
	ICLData data;

	data.length = bufferLengthBytes;
	data.value = pBuffer;

	ICL_SHABegin(&SHAState);
	ICL_SHAProcess(&data, &SHAState);
	ICL_SHAEnd(&SHAState, digest);
}


/* Generate random u value in range (0, max) */
void
RNGf(
	  W8 *r,            /* OUT: 160 bit result */ 
	  W8 *max           /* IN: 160 bit exclusive max */
	  )
{
	int length=0;
	W8 zero[20];

	/*  the number of bits used in max */
	int nbits=0;
	W8 n;
	int i;
    for (i=0;i<20;i++)
	{
		n = max[19-i];
		if (n){
			nbits = i*8;
			while(n != 0)
			{
				nbits++;
				n >>= 1;
			}
		}
	}

	length = (nbits + 7) / 8;
	memset(zero,0,20);
	do{
	memset(r, 0, 20);
	Rng_GetRandomNumber(r, length);
	r[length-1] &= 0xff >> (8 - nbits % 8);
	bswap160(r,r);
	}while( Compare(r,max)==1 || Compare(r,zero)==0) ;
	
}


void
ecPtAdd(
		W8 *R,          /* OUT: 320 bit ec point */
		W8 *P,          /* IN: 320 bit ec point */
		W8 *Q,          /* IN: 320 bit ec point */
		W8 *a,          /* IN: 160 bit ec parameter */
		W8 *p           /* IN: 160 bit ec field prime */ 
		)
{
	/*assuption: p > 3 */
    W8 Infinity[POINTSIZE_BYTES];
	W8 Three[POINTSIZE_BYTES/2];
	W8 Two[POINTSIZE_BYTES/2];
	W8 *ZERO=Infinity;
	W8 s[POINTSIZE_BYTES/2];
	W8 tempResult[POINTSIZE_BYTES];
	
	W8 *xP = P;
	W8 *yP = P + POINTSIZE_BYTES/2;
	W8 *xQ = Q;
	W8 *yQ = Q + POINTSIZE_BYTES/2;
	W8 *xR = tempResult;	
	W8 *yR = tempResult + POINTSIZE_BYTES/2;
		
	memset (Infinity, 0, POINTSIZE_BYTES);
	memset (Three, 0, POINTSIZE_BYTES/2);
	memset (Two, 0, POINTSIZE_BYTES/2);

	// This is Little endian.. change if needed!!
	Three[19]=0x03;
    Two[19]=0x02;



	/*If P or Q infinity, result Q or P*/
	if (isInfinity(P)) {
		memmove(R,Q,POINTSIZE_BYTES);
		return;
	}
	else if (isInfinity(Q)) {
		memmove(R,P,POINTSIZE_BYTES);
		return;
	}
	else {
		if (0==Compare(xP, xQ)) /* if xP == xQ*/
		{
			if (0!=Compare(yP,yQ) || 0==Compare(yP,ZERO)) /* if yP!=yQ or yP==0*/
			{
				/* return point at invinity for sum of inverses */			
				// QUESTION: If point compression is not used to we need to explicitly check yP=-yQ?
				memcpy(R,Infinity,POINTSIZE_BYTES);
				return;
			}
			else /* cant calc s the normal way (yP=yQ), use point doubling */
			{
				W8 temp[POINTSIZE_BYTES/2];
				/* START s= (3 * xP^2 + a)/2*yP mod p */
				ModMul(s,xP,xP,p);
				ModMul(s,Three,s,p);
				ModAdd(s,s,a,p);
                ModMul(temp,Two,yP,p);
				ModInv(temp,temp,p);
				ModMul(s,s,temp,p);
				/* END s= (3 * xP^2 + a)/2*yP mod p*/
			} /* else (yP==yQ) and yP!=0 */
		} /* if no inverse */
		else /* there is an inverse calculate s */
		{
			W8 temp[POINTSIZE_BYTES/2];
			/*START s = (yP - yQ) / (xP - xQ) mod p */
			ModSub(s,yP,yQ,p);
			ModSub(temp,xP,xQ,p);
			ModInv(temp,temp,p);
			ModMul(s,s,temp,p);
			/*END s = (yP - yQ) / (xP - xQ) mod p */
		}
		/*START xR = s2 - xP - xQ mod p */
		ModMul(xR,s,s,p);
		ModSub(xR,xR,xP,p);
		ModSub(xR,xR,xQ,p);
		/*END xR = s2 - xP - xQ mod p */	
		
		/*START yR = s(xP - xR) - yP mod p */
		ModSub(yR,xP,xR,p);
		ModMul(yR,s,yR,p);
		ModSub(yR,yR,yP,p);
		/*END yR = -yP + s(xP - xR) mod p */

		//copy to actual result (which may be same as operands)
		memcpy(R,tempResult,POINTSIZE_BYTES);
	}
}


#define TOPSHIFT  (8*sizeof(W32)-1)
#define TOPMASK (1UL << TOPSHIFT)
// clength = 160bits, 20 bytes = 5 * sizeof (W32)
#define CLENGTH 20 

void
ecPtMultScalar(
			   W8* R,          /* OUT: 320 bit ec point */
			   W8* k,          /* IN: 160 bit constant */
		       W8* P,          /* IN: 320 bit ec point */	
		       W8* a,          /* IN: 160 bit ec parameter */
		       W8* p           /* IN: 160 bit ec field prime */ 
			   )
{
	W8 mask;
    int i;
	//----------
	int total_doubles=0;
	int total_adds=0;
	//----------
	//W8 tmpK[20];
	W8 Infinity[POINTSIZE_BYTES];
	memset (Infinity,0,POINTSIZE_BYTES);
	memcpy(R,Infinity,POINTSIZE_BYTES); /*set result to infinity*/
	//bswap160(k,(W8 *)tmpK);

	for (i = 0; i <20; i++) {
		for (mask = 0x80; mask; mask >>= 1) {			
			/*result += result*/
			ecPtAdd(R,R,R,a,p); 
			//----------
			//total_doubles++;
			//printf("\ndouble # %d", total_doubles);
			//if (8==total_doubles)
			//{
			//	char sresultX[41];
			//	char sresultY[41];
			//	stringify(R,sresultX, 20);
			//	stringify((R + 20),sresultY, 20);
			//	printf ("double result is : (\n%s\n , \n%s\n)  i=%d\n",sresultX,sresultY,i);
			//}
			//----------
			if (mask & k[i]) {
				char sresultX[41];
				char sresultY[41];
                sresultX[40]= 0;			
				sresultY[40]= 0;
				/*result +=P */
				ecPtAdd(R,R,P,a,p);
				//----------
                //stringify(R,sresultX, 20);
				//stringify((R + 20),sresultY, 20);
				//total_adds++;
				//printf("\nadd # %d", total_adds);
				//printf ("add result is : (\n%s\n , \n%s\n)\n",sresultX,sresultY);
			    //----------
			}
		}
	}
} // ecPtMultScalar

/*Compare 2 160 bit (unsigned) numbers, return 1 if a>b, -1 if a<b 0 if a==b*/
int 
Compare
  (
   W8 *a,         /* IN: 160 bit a */
   W8 *b          /* IN: 160 bit b */
   )
{
	int i;
	for (i=0;i<20;i++)
	{
		if (a[i] > b[i]) {
			return 1;
		}
		if (a[i] < b[i]) {
			return -1;
		}
	}
	return 0;
}

int
isInfinity
  (
   W8 *aECpt  /* IN: 320 bit ECpt */
   )
{
	/*Assuming 0 0 is infinity for now, may need to change*/
	int i;
	for (i=0;i<10;i++)
	{
		if (((W32 *)aECpt)[i]) {
			return 0;
		}
	}
	return 1;
}


void
Mod(
	W8 *r,            /* OUT: 160 bit result */
	W8 *x,            /* IN: 160 bit x */
	W8 *m             /* IN: 160 bit modulus */
	)
{
    RSAInt ai;  // these are pre initialzed with padding
	RSAInt ni;
	RSAInt ti;

	RSAData a;
	RSAData n;
	RSAData t;
	// for modinv lengths are byte lengths
	a.length=5;
	a.value=ai.value;
	n.length=5;
	n.value=ni.value;
	t.length=5;
	t.value=ti.value;

	
	a.value=bswap160(x,a.value);
	n.value=bswap160(m,n.value);

	ICL_Rem (&a, &n, &t);
	bswap160(t.value,r);
}

void
ModAdd(
	   W8 *r,            /* OUT: 160 bit result */ 
	   W8 *x,            /* IN: 160 bit x */ 
	   W8 *y,            /* IN: 160 bit y */
	   W8 *m             /* IN: 160 bit moduus */
	   )
{
	RSAInt ai;  // these are pre initialzed with padding
	RSAInt bi;
	RSAInt ni;
	RSAInt ti;

	RSAData a;
	RSAData b;
	RSAData n;
	RSAData t;
	// for modinv lengths are byte lengths
	//a.length=5;
	a.value=ai.value;
	//b.length=5;
	b.value=bi.value;
	//n.length=5;
	n.value=ni.value;
	t.length=5;
	t.value=ti.value;
	w32WordLength160(x,&a);
	w32WordLength160(y,&b);
	w32WordLength160(m,&n);

	

	
	a.value=bswap160(x,a.value);
	b.value=bswap160(y,b.value);
	n.value=bswap160(m,n.value);

	ICL_ModAdd(&a, &b, &n, &t);
	ICL_Padding0(&t);
	bswap160(t.value,r);
}



void
ModSub(
	   W8 *r,            /* OUT: 160 bit result */ 
	   W8 *x,            /* IN: 160 bit x */ 
	   W8 *y,            /* IN: 160 bit y */
	   W8 *m             /* IN: 160 bit moduus */
	   )
{
    RSAInt ai;  // these are pre initialzed with padding
	RSAInt bi;
	RSAInt ni;
	RSAInt ti;

	RSAData a;
	RSAData b;
	RSAData n;
	RSAData t;
	// for modinv lengths are byte lengths
	//a.length=5;
	a.value=ai.value;
	//b.length=5;
	b.value=bi.value;
	//n.length=5;
	n.value=ni.value;
	t.length=5;
	t.value=ti.value;
	w32WordLength160(x,&a);
	w32WordLength160(y,&b);
	w32WordLength160(m,&n);

	
	a.value=bswap160(x,a.value);
	b.value=bswap160(y,b.value);
	n.value=bswap160(m,n.value);	
	
	ICL_ModSub(&a, &b, &n, &t);
	ICL_Padding0(&t);
	bswap160(t.value,r);

}	   
void
ModMul(
	   W8 *r,            /* OUT: 160 bit result */ 
	   W8 *x,            /* IN: 160 bit x */ 
	   W8 *y,            /* IN: 160 bit y */
	   W8 *m             /* IN: 160 bit modulus */
	   )
{
	RSAInt ai;  // these are pre initialzed with padding
	RSAInt bi;
	RSAInt ni;
	RSAInt ti;

	RSAData a;
	RSAData b;
	RSAData n;
	RSAData t;
	// for modinv lengths are byte lengths
	//a.length=5;
	a.value=ai.value;
	//b.length=5;
	b.value=bi.value;
	//n.length=5;
	n.value=ni.value;
	t.length=5;
	t.value=ti.value;
	w32WordLength160(x,&a);
	w32WordLength160(y,&b);
	w32WordLength160(m,&n);

	//memset(a.value,0,MODULUSBYTES);
	//memset(b.value,0,MODULUSBYTES);
	//memset(n.value,0,MODULUSBYTES);
	//memset(t.value,0,MODULUSBYTES);
	
	a.value=bswap160(x,a.value);
	b.value=bswap160(y,b.value);
	n.value=bswap160(m,n.value);
 
	ICL_ModMul(&a, &b, &n, &t);
	ICL_Padding0(&t);
	bswap160(t.value,r);

}

void
ModInv(
	   W8 *r,            /* OUT: 160 bit result */ 
	   W8 *x,            /* IN: 160 bit x */ 
	   W8 *m             /* IN: 160 bit modulus */
	   )
{
	RSAInt ai;  // these are pre initialzed with padding
	RSAInt ni;
	RSAInt ti;

	RSAData a;
	RSAData n;
	RSAData t;
	// for modinv lengths are byte lengths
	byteLength160(x,&a);
    //a.length=1;
	a.value=ai.value;
	byteLength160(m,&n);
	//n.length=1;
	n.value=ni.value;
	t.length=20;
	t.value=ti.value;

	//memset(a.value,0,MODULUSBYTES);
	//memset(n.value,0,MODULUSBYTES);
	//memset(t.value,0,MODULUSBYTES);
	
	a.value=bswap160(x,a.value);	
	n.value=bswap160(m,n.value);
	
	
	// ICL Functions DESTROY INPUTS!!!
	ICL_ModularInverse(&a, &n, &t);
	ICL_Padding0(&t);
	bswap160(t.value,r);
}
#if 0
void
ModDiv(
	   W8 *r,            /* OUT: 160 bit result */ 
	   W8 *x,            /* IN: 160 bit x */ 
	   W8 *y,            /* IN: 160 bit y */
	   W8 *m             /* IN: 160 bit modulus */
	   )
{

	//y/x mod M  x and M must be relatively prime (gcd (x,m) =1)
	//U is division result
	//U/2 = U>>1
	//carry = 0;
	//for (i=0;i<20;i++)
	//{
	//	byte=U[i];		
	//	U[i] = (byte >>1 ) ^ (carry<<7);
	//	carry =(byte & 0x01);
	//}
	//
	//U+M = U xor M
	// has issues

	A=x;
	B=m;
	U=y;
	V=0;
	while (A!=B)
	{
		if (even(A)) 
		{
			A=A/2;
			U = (even(U))? U/2:(U+M)/2;
		} 
		else if (even(B)) 
		{
			B=B/2;
			V = (even(V))? V/2:(V+M)/2;
		}
		else if (A>B) 
		{
			A=(A-B)/2;
			U=U-V;  
			if (U<0)
			{
				U=U+M;
			}
			U= (even(U))? U/2:(U+M)/2;
		}
		else
		{
			B=(B-A)/2;
			V=V-U;
			if (V<0)
			{
				V=V+M;
			}
			V= (even(V))? V/2:(V+M)/2;
		}

	}

}
#endif// 0