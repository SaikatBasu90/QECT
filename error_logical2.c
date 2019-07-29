/*
Author: Priyanka Mukhopadhyay & Saikat Basu
Affiliation: Indian Statistical Institute, Kolkata
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

FILE *fp, *fp1;
struct Qubit
{
	char *name;
	int length;
	float err;
	struct Gate *next;
}; //Structure to point to the qubit lines

struct Gate
{
	int type;
	int num_edge;
	int edge[3];
	char *name;
	struct Qubit *prev[3];
	struct Gate *next[3], *prev1[3];
	float in_err[6];
	float fin_err[6];
}; //Structure for each gate

int numQu, pmd, concat, count = 0; // numQu = #qubits; concat = concatenation level
float gate[13], Rx[7], Ry[7], Rz[7]; //Worst gate error for specific PMD
float thresh, qecc = 0, prep = 0;
char qecc_pmd[10];
struct Qubit *qubits;
struct Gate *gates;

void buildQubit(void);
void buildGate(void);
int stringLen(char *);
int stringComp(char *, char *, int);
int searchQu(char *, int);
void append(int, int);
float calcErr(int, int, int, int);
void calcGateErr(void);
void printFin(void);
void criticalPath(void);
int calcMaxErr(void); 
void backTrace(struct Gate *);



int main(int argc, char *argv[])
{
char s[50];
const char s1[7] = ".qubit";
char *token;
int i;
if (argc<4)
{
  printf("\n\n \t ======= Quantum Error Correction Tool ====== \n \n \t Programmer:Saikat Basu \t and \t  Priyanka Mukhopadhyay \n\t                 Indian Statistical institute ");
  printf("\n\n Format :: <Input file name> <Output file name> <Threshold> <Level of Concatenation> <QECC_PMD.txt>\n");
}
else
{  
  fp = fopen(argv[1], "r");
  if(fp == NULL)
  {
	printf("\n File cannot be opened.\n");
	exit(1);
  }
  else
  {
	printf("\n File opened successfully.\n");
	concat=atoi(argv[4]);
	printf("\n You have considered %d levels of concatenation. \n", concat);
	
	fp1 = fopen(argv[2], "w");
	if(fp1 == NULL)
		printf("\nOutput file cannot be opened.\n");
	
	fgets(s, 50, fp);	
	
	while((s[0] != '.')|(s[1] != 'q')|(s[2] != 'u')|(s[3] != 'b')|(s[4] != 'i')|(s[5] != 't'))
	
	{
		fgets(s, 45, fp);
	
	}
	
	fprintf(fp1, " # # Created by Priyanka Mukhopadhyay and Saikat Basu # #\n------------------------------------------------------- \n");
	fprintf(fp1, "%s \n", s);
	fprintf(fp1, "Number of levels of concatenation is %d. \n", concat);
	
	
	strcpy(qecc_pmd,argv[5]);
	printf("qecc and pmd file to be opened %s \n", qecc_pmd);
	
	
	calcGateErr();
	
	thresh=atof(argv[3]);
	printf("\nThreshold  = %f. \n", thresh);

	token = strtok(s, s1);
	
	numQu = atoi(token);
	printf("\n Number of qubit lines is %d.\n", numQu);
	
	
	
	
	buildQubit();

	buildGate();
	fprintf(fp1, "\n Final errors in respective qubit lines are:\n");
	printFin();
	printf("\n No of QECC = %d. \n", count);
	fprintf(fp1, "\n Critical Path is: \n");
	criticalPath();
	
	fclose(fp);
	fclose(fp1);
	
  }
}

return 0;
}

//-----------Reading from file gate error at logic level--------------
/*
	This module reads gate error probability from a specific file. You have to give the file name accordingly.
*/

void calcGateErr(void)
{
	int i, level;
	char s[500];

	FILE *fp2;
	
	fp2 = fopen(qecc_pmd,"r");	//Give the file name accordingly.
	if(fp2 == NULL)
	{
		printf("\n Cannot read gate errors. \n");
		exit(2);
	}
	else
	{
		fgets(s, 500, fp2);
		while((s[0] != 'G')&(s[1] != 'a')&(s[2] != 't')&(s[3] != 'e')&(s[4] != 's'))
			fgets(s, 500, fp2);
		for(i = 0; i < (concat+1); i++)
			fgets(s, 500, fp2);
		fscanf(fp2, "%d", &level);
		for(i = 0; i < 13; i++)
			fscanf(fp2, "%f", &gate[i]);
		printf("\n Level of encoding = %d. \n", level);
		printf("\n The error probability of gates are: \n");
		for(i = 0; i < 13; i++)
			printf(" %f   ", gate[i]);
		
		fgets(s, 500, fp2);
		while((s[0] != 'R')&(s[1] != 'x'))
		{	fgets(s, 500, fp2);}
		for(i = 0; i < (concat+1); i++)
		{	fgets(s, 500, fp2);}
		fscanf(fp2, "%d", &level);
		for(i = 0; i < 7; i++)
		{	fscanf(fp2, "%f", &Rx[i]);}
		printf("\n Level of encoding = %d. \n", level);
		printf("\n The error probability of Rx are: \n");
		for(i = 0; i < 7; i++)
		{	printf(" %f   ", Rx[i]);}	
			
		fgets(s, 500, fp2);
		while((s[0] != 'R')|(s[1] != 'y'))
			fgets(s, 500, fp2);
		for(i = 0; i < (concat+1); i++)
			fgets(s, 500, fp2);
		fscanf(fp2, "%d", &level);
		for(i = 0; i < 7; i++)
			fscanf(fp2, "%f", &Ry[i]);
		printf("\n Level of encoding = %d. \n", level);
		printf("\n The error probability of Ry are: \n");
		for(i = 0; i < 7; i++)
			printf(" %f   ", Ry[i]);
			
		fgets(s, 500, fp2);
		while((s[0] != 'R')|(s[1] != 'z'))
			fgets(s, 500, fp2);
		for(i = 0; i < (concat+1); i++)
			fgets(s, 500, fp2);
		fscanf(fp2, "%d", &level);
		for(i = 0; i < 7; i++)
			fscanf(fp2, "%f", &Rz[i]);
		printf("\n Level of encoding = %d. \n", level);
		printf("\n The error probability of Rz are: \n");
		for(i = 0; i < 7; i++)
			printf(" %f   ", Rz[i]);
	}
	
return;
}

//----------Build the data structure for qubit lines--------------
void buildQubit(void)
{	
	char s[50], s1[] = " ";	
	int i, j, len, size;
	
	qubits = malloc(numQu*sizeof(struct Qubit));

	fgets(s, 50, fp);	
	
	while((s[0] != 'q')|(s[1] != 'u')|(s[2] != 'b')|(s[3] != 'i')|(s[4] != 't'))
	{
		fgets(s, 50, fp);	
	}

	for(i = 0; i < numQu; i++)
	{
		fprintf(fp1, "%s", s);
		len = stringLen(s);

		size = len - 6;
		qubits[i].name = malloc((size+5)*sizeof(char));
		for(j = 0; j < size; j++)
		{
			qubits[i].name[j] = s[6+j];
		}
		qubits[i].name[j] = '\0';
		
		qubits[i].next = NULL;
		qubits[i].length = stringLen(qubits[i].name);		
		qubits[i].err = prep;
		fgets(s, 50, fp);
	}
	
	fgets(s, 20, fp);

	fprintf(fp1, "\n");
return;
}

//-------------Claculate string length----------------

int stringLen(char *s)
{
	int len = 0;
	while((*s != '\0')&&(*s != '\n')&&(*s != '\r'))
	{
		s++;
		len++;
	}	
return(len);
}

//------------------Building gates and error tracing-----------------

void buildGate(void)
{
	int i, j, len, indx, dec, max_line, phase;
	float err, max_err;
	char s[20], s1[] = " ";	
	char *token;
	char ch, axis, temp;

	fgets(s, 20, fp);	
	token = ".end";
	dec=stringComp(s, token, 4);
	while(dec != 1)
	{
		fprintf(fp1, "%s", s);
		
		token = strtok(s, s1);	
		ch = token[0];	
		gates = malloc(1*sizeof(struct Gate));
		gates->prev[0] = NULL; gates->prev[1] = NULL; gates->prev[2] = NULL;
		gates->prev1[0] = NULL; gates->prev1[1] = NULL; gates->prev1[2] = NULL;
		gates->next[0] = NULL; gates->next[1] = NULL; gates->next[2] = NULL;
		gates->in_err[0]=0; gates->in_err[1]=0; gates->in_err[2]=0; gates->in_err[3]=0; gates->in_err[4]=0; gates->in_err[5]=0;
		gates->fin_err[0]=0; gates->fin_err[1]=0; gates->fin_err[2]=0; gates->fin_err[3]=0; gates->fin_err[4]=0; gates->fin_err[5]=0;
		max_err = 0;
		switch(ch)
		{
		case('X'):token = strtok(NULL, s1);	
				len = stringLen(token);
				indx = searchQu(token, len);	
				gates->type = 0;	gates->num_edge = 1;	gates->name = "X";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('Y'):token = strtok(NULL, s1);	
				len = stringLen(token);
				indx = searchQu(token, len);	
				gates->type = 1;	gates->num_edge = 1;	gates->name = "Y";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('Z'):token = strtok(NULL, s1);	
				len = stringLen(token);	
				indx = searchQu(token, len);	
				gates->type = 2;	gates->num_edge = 1;	gates->name = "Z";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('R'):axis = token[1];	token = strtok(NULL, s1);	
				len = stringLen(token);
				indx = searchQu(token, len);	
				gates->num_edge = 1;	gates->edge[0] = indx;
				
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				token = strtok(NULL, s1);	phase = atoi(token);
				if(phase < 0)
					phase = 8 + phase;
				if(axis == 'X')
				{
					gates->type = 13;	gates->name = "RX";
				}
				if(axis == 'Y')
				{
					gates->type = 14;	gates->name = "RY";
				}
				if(axis == 'Z')
				{
					gates->type = 15;	gates->name = "RZ";
				}
				err = calcErr(gates->type,indx, phase-1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('T'):token = strtok(NULL, s1);	
				len = stringLen(token);	
				indx = searchQu(token, len);	
				gates->type = 4;	gates->num_edge = 1;	gates->name = "T";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('H'):token = strtok(NULL, s1);	
				len = stringLen(token);	
				indx = searchQu(token, len);	printf("\n Index is %d.\n", indx);
				gates->type = 5;	gates->num_edge = 1;	gates->name = "H";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
				break;
		case('S'):if(token[1] == 'W')
			{
				gates->type = 6;	gates->num_edge = 2;	gates->name = "SWAP";
				for(i = 0; i < 2; i++)
				{
					token = strtok(NULL, s1);	
					len = stringLen(token);
					indx = searchQu(token, len);	
					gates->edge[i] = indx;
					gates->in_err[2*i] = qubits[indx].err;
					append(indx, i);
				}
				gates->edge[2] = -1;
				if((gates->in_err[0]) > (gates->in_err[2]))
				{
					err = calcErr(gates->type, gates->edge[0], gates->edge[1], -1 );
					gates->in_err[1] = 1;
				}
				else if ((gates->in_err[0]) == (gates->in_err[2]))
				{
					err = calcErr(gates->type, gates->edge[0], gates->edge[1], -1 );
					gates->in_err[1] = 1;	gates->in_err[3] = 1;
				}
				else
				{
					err = calcErr(gates->type, gates->edge[1], gates->edge[0], -1);
					gates->in_err[3] = 1;
				}
				gates->fin_err[0] = err; 	gates->fin_err[1] = 1;
				gates->fin_err[2] = err;	gates->fin_err[3] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f and %s : %f.\n",qubits[gates->edge[0]].name, err, qubits[gates->edge[1]].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[gates->edge[0]].err = qecc;	qubits[gates->edge[1]].err = qecc;
					gates->fin_err[0] = qecc; 	gates->fin_err[1] = 1;
					gates->fin_err[2] = qecc;	gates->fin_err[3] = 1;
				}
			}
			else
			{
				token = strtok(NULL, s1);	
				len = stringLen(token);
				indx = searchQu(token, len);	
				gates->type = 3;	gates->num_edge = 1;	gates->name = "S";
				gates->edge[0] = indx;
				gates->in_err[0] = qubits[indx].err;	gates->in_err[1] = 1;
				append(indx, 0);
				gates->edge[1] = -1;	gates->edge[2] = -1;
				err = calcErr(gates->type,indx, -1, -1);
				gates->fin_err[0] = err;	gates->fin_err[1] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f.\n",qubits[indx].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[indx].err = qecc;
					gates->fin_err[0] = qecc;	gates->fin_err[1] = 1;
				}
			}
			break;
	case('C'):temp = token[1];	gates->num_edge = 2;
				for(i = 0; i < 2; i++)
				{
					token = strtok(NULL, s1);	
					len = stringLen(token);	
					indx = searchQu(token, len);	
					gates->edge[i] = indx;
					gates->in_err[2*i] = qubits[indx].err;
					append(indx, i);
				}
				gates->edge[2] = -1;
				if((temp == 'N')||(temp == 'n'))
				{
					gates->type = 7;	gates->name = "CNOT";
				}
				else
				{
					gates->type = 8;	gates->name = "CZ";
				}
				if((gates->in_err[0]) > (gates->in_err[2]))
				{
					err = calcErr(gates->type, gates->edge[0], gates->edge[1], -1 );
					gates->in_err[1] = 1;
				}
				else if ((gates->in_err[0]) == (gates->in_err[2]))
				{
					err = calcErr(gates->type, gates->edge[0], gates->edge[1], -1 );
					gates->in_err[1] = 1;	gates->in_err[3] = 1;
				}
				else
				{
					err = calcErr(gates->type, gates->edge[1], gates->edge[0], -1);
					gates->in_err[3] = 1;
				}
				gates->fin_err[0] = err; 	gates->fin_err[1] = 1;
				gates->fin_err[2] = err;	gates->fin_err[3] = 1;
				fprintf(fp1, "Prob Error on qubit line %s : %f and %s : %f.\n",qubits[gates->edge[0]].name, err, qubits[gates->edge[1]].name, err);
				if(err > thresh)	//Place QECC if threshold is exceeded
				{
					count = count + 1;
					fprintf(fp1, "%d QECC INSERTED. \n", count);
					qubits[gates->edge[0]].err = qecc;	qubits[gates->edge[1]].err = qecc;
					gates->fin_err[0] = qecc; 	gates->fin_err[1] = 1;
					gates->fin_err[2] = qecc;	gates->fin_err[3] = 1;
				}
				break;
		default:
				break;
		
		}
	fgets(s, 20, fp);	
	token = ".end";
	dec=stringComp(s, token, 4);
	}

return;
}

//-------------Comparing two strings. Return 1 if matches, else 0---------------------------

int stringComp(char *name, char *token, int len)
{
	int dec = 1, i; 
	for(i = 0; i < len; i++)
	{
		if(name[i] == token[i])
			dec = dec*1;
		else dec = dec*0;
	}
	
return(dec);
}

//---------------Search on which qubit(s) gate operates-------------

int searchQu(char *token, int len)
{
	int i, indx, dec;
	
	for(i = 0; i < numQu; i++)
	{
		if(len == qubits[i].length)
		{
			dec = stringComp(qubits[i].name, token, len);
			if(dec == 1)
			{
				break;
			}				
		}
	}

return(i);
}

//--------------Append the node corresponding to a gate on the link list-------------------

void append(int indx, int i)
{
	int which;
	struct Gate *curr;

	//printf("\nAppend start\n");
	if(qubits[indx].next == NULL)	
	{
		gates->prev[i] = &qubits[indx];
		qubits[indx].next = gates;
		//printf("\nFirst if\n");
	}
	else
	{
		curr = qubits[indx].next;
		for(which=0; which<3; which++)
		{
			if(curr->edge[which] == indx)
				break;
		}
		while(curr->next[which] != NULL)
		{
			curr = curr->next[which];
			for(which=0; which<3; which++)
			{
			if(curr->edge[which] == indx)
				break;
			}
//			printf("\nNext pointer = %d \n",curr->next[which]);
		}
		gates->prev1[i] = curr;
		curr->next[which] = gates;
	}
//	printf("\nAppend end\n");
return;
}

//------------------Error calculation sub-routine---------------

float calcErr(int type,int indx, int indx1, int indx2)
{
	float err;
	
	if(type < 13)
	{
		err = qubits[indx].err + gate[type];//gateErr[pmd - 1]; //+ errProb[type-1];
		qubits[indx].err = err;
		if((type == 7)||(type == 8)||(type == 9))
			qubits[indx1].err = err;
		if(type == 10)
		{
			qubits[indx1].err = err;
			qubits[indx2].err = err;
		}
	}
	if(type == 13)	
	{
		err = qubits[indx].err + Rx[indx1];
		qubits[indx].err = err;
	}
	if(type == 14)	
	{
		err = qubits[indx].err + Ry[indx1];
		qubits[indx].err = err;
	}
	if(type == 15)	
	{
		err = qubits[indx].err + Rz[indx1];
		qubits[indx].err = err;
	}
	
return(err);
}

//----------------Print final errors on qubits in file-------------------

void printFin(void)
{
	int i;
	
	for(i = 0; i < numQu; i++)
	{
		fprintf(fp1, "%s : %f \n", qubits[i].name, qubits[i].err);
	}
	
return;
}

//----------------Trace critical path-------------------------

void criticalPath(void)
{
	int i, j, max_line;
	struct Gate *curr;

	max_line = calcMaxErr();
	fprintf(fp1, "Maximum prob of error on qubit line %s. \n", qubits[max_line].name);

	curr = qubits[max_line].next;
	for(i = 0; i < (curr->num_edge); i++)
		if(curr->edge[i] == max_line)
			break;
	while(curr->next[i] != NULL)
	{
		curr = curr->next[i];
		for(i = 0; i < (curr->num_edge); i++)
		if(curr->edge[i] == max_line)
			break;
	}
	fprintf(fp1, "%s (%s) ", curr->name, qubits[curr->edge[i]].name);
	for(j = 0; j < (curr->num_edge); j++)
		fprintf(fp1, "%s (Initial err = %f)", qubits[curr->edge[j]].name, curr->in_err[2*j]);
	fprintf(fp1, " Final err = %f ", curr->fin_err[0]);

	backTrace(curr);

return;
}

//------------------Calculate on which qubit is the maximum error---------------------

int calcMaxErr(void)
{
	int i, indx;
	float max = 0;
	
	for(i = 0; i < numQu; i++)	
	{
		if(qubits[i].err >= max)
		{
			max = qubits[i].err;
			indx = i;
		}
	}

return(indx);
}

//-------------Backtracing to find critical path--------------------

void backTrace(struct Gate *curr)
{
	int i, j;
	
	for(i = 0; i < 3; i++)
	{
		if(curr->in_err[2*i + 1] == 1)
		{
			fprintf(fp1, " Max input error along %s \n", qubits[curr->edge[i]].name);
			if(curr->in_err[2*i] == 0)
				exit(3);
			else
			{
				curr = curr->prev1[i];	
				fprintf(fp1, "%s  ", curr->name);
				for(j = 0; j < (curr->num_edge); j++)
					fprintf(fp1, "%s (Initial err = %f)", qubits[curr->edge[j]].name, curr->in_err[2*j]);
				fprintf(fp1, " Final err = %f ", curr->fin_err[0]);
				backTrace(curr);
			}
		}
	}

return; 
}
