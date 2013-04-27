#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

//definice struktury pro uchov�n� matice
typedef struct
{
    unsigned int cols;
    unsigned int rows;
    char **arr;
} Tmatrix;

//definice typu pro ukl�d�n� sou�adnic pole
typedef struct
{
    int row;
    int col;
} Tcoord;

void printResult(Tmatrix* m);
int solve(Tmatrix* m, char* wordsFile);
bool findWord(char* searchWord, Tmatrix* matrix);
void printMatrix(Tmatrix* m);
char upperChar(char charCode);
char lowerChar(char charCode);
int getParam(char* param);
int matrixFill(char* filename, Tmatrix* m);
bool isValidChar(char* loadedStr, char* charCode);
void matrixFree(Tmatrix* m);
Tmatrix matrixAlloc(unsigned int rows, unsigned int cols);
void highlighWord(Tmatrix* m, Tcoord* c, int coordsCount);
int matrixFind(Tmatrix* m, char* searchWord, int searchWordSize);
void printError(int errType);
void printHelp(void);

//definice hodnot chybov�ch k�d�
enum ERRORCODES
{
    EPARAM,
    EFOPEN,
    ESTRUCTURE,
    EWORDNOTFOUND,
    ESOLVE,
    EMALLOC,
};

//definice hodnot parametr�
enum PARAMCODES 
{
    TEST,
    SEARCH,
    SOLVE,
    HELP,
    NOPARAM,
};

//definice parametr�
const char* PARAMS[] =
{
    [TEST] = "--test",
    [SEARCH] = "--search=",
    [SOLVE] = "--solve",
    [HELP] = "-h",
};

//definice chybov�ch hl�en�
const char* ERRORS[] = 
{
    [EPARAM] = "�patn� parametry",
    [EFOPEN] = "Soubor se nepoda�ilo otev��t",
    [ESTRUCTURE] = "Soubor m� �patn� form�t",
    [EWORDNOTFOUND] = "Slovo nenalezeno",
    [ESOLVE] = "Nepoda�ilo se vylu�tit tajenku, slovo nenalezeno",
    [EMALLOC] = "Chyba p�i alokaci pam�ti",
};

const char CH = '#'; //hodnota p�smena ch v matici
const char bigCH = '$'; //velk� ch
const int ESTRUCTUREFAIL = -1;
const int EFOPENFAIL = -2;
const int ESOLVEFAIL = -3;
const int EMALLOCFAIL = -4;
const int waysCount = 8;

//v�pis n�pov�dy
void printHelp(void)
{
    printf("%s",
	   	"N�ZEV\n"
		"	proj3\n"
		"AUTOR\n"
		"	Luk� Vokr��ko\n"
		"POU�IT�\n"
		"	proj3 --test|--search=slovo|--solve|--help [zdrojov� soubor] [soubor se slovy]\n"
		"PARAMETRY\n"
		"	-h - tiskne n�pov�du\n"
		"	--test - otestuje a vytiskne osmism�rku\n"
		"	--search=slovo - najde a zv�razn� hledan� slovo\n"
		"	--solve - vy�e�� osmism�rku a vytiskne v�sledek\n"
	   );    
}

//v�pis chyb
//errType index chybov� zpr�vy
void printError(int errType)
{
    fprintf(stderr, "%s\n", ERRORS[errType]);
}

//hled�n� slova s jeho vyzna�en�m
//m odkaz na matici
//searchWord pole k�d� pro p�smena
//searchWordSize velikost pole
int matrixFind(Tmatrix* m, char* searchWord, int searchWordSize)
{
    //deklarace v�ech sm�r�
    Tcoord directions[8] = {{-1,0}, {-1,+1}, {0,+1}, {+1,+1}, {+1,0}, {+1,-1}, {0,-1}, {-1,-1}};
    Tcoord* singleCoords = malloc(sizeof(Tcoord)*searchWordSize);
    int founds = 0;
    char charCode;
    
    //hled�n� prvn�ho znaku
    for(unsigned i = 0; i < m->cols*m->rows; ++i)
    {
	//v�po�et index�
	unsigned int row = i/m->cols;
	unsigned int col = i%m->cols;

	charCode = lowerChar(searchWord[0]);
	//prvn� znak nalezen
	if(charCode == lowerChar(m->arr[row][col]))
	{
	    //ulo��m jeho pozici abych ho pot� mohl zv�raznit
	    singleCoords[0].row = row;
	    singleCoords[0].col = col;
	    
	    unsigned int newRow = row;
	    unsigned int newCol = col;
	    
	    //zkou��m sm�ry
	    for(int j = 0; j < waysCount; ++j)
	    {
		newRow = row;
		newCol = col;
		//kontrola znak po znaku
		for(int k = 1; k < searchWordSize; ++k)
		{
		    //posunu se o jeden znaka ve sm�ru
		    newRow = newRow + directions[j].row;
		    newCol = newCol + directions[j].col;
		    
		    //kontrola, jestli se nov� sou�adnice nach�z� v matici
		    if(newCol < m->cols && newRow < m->rows)
		    {
			charCode = lowerChar(searchWord[k]);
			//odpov�d� dal�� p�smeno?
			if(charCode == lowerChar(m->arr[newRow][newCol]))
			{
			    //ulo� jeho sou�adnice pro pozd�j�� zv�razn�n�
			    singleCoords[k].row = newRow;
			    singleCoords[k].col = newCol;
			    
			    //bylo to posledn� p�smeno?
			    if(k == searchWordSize-1) 
			    {
				++founds;
				highlighWord(m, singleCoords, searchWordSize);
			    }
			    //a pokra�uj hled�n�m dal��ho
			    else continue;
			}
			else break;
		    }
		    //pozice je mimo matici nebo se znaky neshoduj�, nen� t�eba zku�et dal��
		    break;
		}
	    }
	}
    }
        
    return founds;
}

//p�eveden� znak� na velk� p�smena
//m dokaz na matici
//c pole index�, kde maj�b�t velk� p�smena
//coordsCount - velikost pole
void highlighWord(Tmatrix* m, Tcoord* c, int coordsCount)
{
    char charCode;
    for(int i = 0; i < coordsCount; ++i)
    {
	charCode = m->arr[c[i].row][c[i].col];
	m->arr[c[i].row][c[i].col] = upperChar(charCode);
    }
}

//alokace pam�ti pro matici
//rows po�et ��dk�
//cols po�et sloupc�
Tmatrix matrixAlloc(unsigned int rows, unsigned int cols)
{
    Tmatrix m;
    m.cols = cols;
    m.rows = rows;
    m.arr = malloc(sizeof(char*)*rows);
    if(m.arr == NULL) 
    {
	m.arr = NULL;
	return m;
    }

    for(unsigned int i = 0; i < rows; ++i)
    {
	m.arr[i] = malloc(sizeof(char)*cols);
	if(m.arr[i] == NULL) 
	{
	    m.arr = NULL;
	    break;
	}
    }
    
    return m;
}

//uvoln�n� pam�ti vyhrazen� pro matici
//m odkaz n matici
void matrixFree(Tmatrix* m)
{
    for(unsigned int i = 0; i < m->rows; ++i)
    {
	free(m->arr[i]);
    }
    free(m->arr);
    m->cols = m->rows = 0;
}

//je to p�smeno nebo CH?
//loadedStr - na�ten� p�smeno
//charCode adresa pro ulo�en� znaku
bool isValidChar(char* loadedStr, char* charCode)
{
    int len = strlen(loadedStr);
    
    if(len == 1 && isalpha(loadedStr[0]))
    {
	*charCode = (char) tolower(loadedStr[0]);
    }
    //na�teny 2 znaky
    else if(len == 2 && isalpha(loadedStr[0]) && isalpha(loadedStr[1]))
    {
	loadedStr[0] = tolower(loadedStr[0]);
	loadedStr[1] = tolower(loadedStr[1]);
	
	if(loadedStr[0] == 'c' && loadedStr[1] == 'h') *charCode = CH;
    }
    else
    {
	return false;
    }

    return true;
}

//napln�n� matice ze souboru
//filename n�zev souboru
//m odkaz na matici
int matrixFill(char* filename, Tmatrix* m)
{
    int code, maxNums;
    char charCode;
    int cols, rows;
    char letter[2];
    int readNums = 0;
    FILE* f;

    f = fopen(filename, "r");
    //nepoda�ilo se otev��t soubor
    if(f == NULL) return EFOPENFAIL;

    //na�ten� rozm�r�
    code = fscanf(f, "%d", &rows);
    code += fscanf(f, "%d", &cols);
    //nepovedlo se na��st parametry
    if(code != 2 || cols <= 0 || rows <= 0) return ESTRUCTUREFAIL;

    maxNums = cols*rows;
    *m = matrixAlloc(rows, cols);
    if(m->arr == NULL) return EMALLOCFAIL;

    //�ten� znak�
    while(((code = fscanf(f, "%2s", letter)) != EOF) && readNums != maxNums)
    {
	if((code == 1) && isValidChar(letter, &charCode))
	{
	    m->arr[readNums / m->cols][readNums % m->cols] = charCode;
	}
	//�patn� symbol
	else 
	{
	    break;
	}

	++readNums;
    }
    
    fclose(f);
    
    if(code != EOF || readNums != maxNums) 
    {
	matrixFree(m);
	return ESTRUCTUREFAIL;
    }
    else
    {
	return readNums;
    }
}

//na��t�n� parametr�
//param textov� parametr
int getParam(char* param)
{
    char tmpParam[10];
    memcpy(&tmpParam, param, 9);
    tmpParam[9] = '\0';
    
    if(strcmp(tmpParam, PARAMS[SEARCH]) == 0) return SEARCH;
    else if(strcmp(param, PARAMS[TEST]) == 0) return TEST;
    else if(strcmp(param, PARAMS[SOLVE]) == 0) return SOLVE;
    else if(strcmp(param, PARAMS[HELP]) == 0) return HELP;
    else return NOPARAM;
}

//zm�n�en� znaku
//charCode znak
char lowerChar(char charCode)
{
    if(isalpha(charCode))
    {
	charCode = tolower(charCode);
    }    
    else if(charCode == bigCH)
    {
	charCode = CH;
    }
    
    return charCode;
}

//zv�t�en� znaku
//charCode znak
char upperChar(char charCode)
{
    if(isalpha(charCode))
    {
	charCode = toupper(charCode);
    }    
    else if(charCode == CH)
    {
	charCode = bigCH;
    }
    
    return charCode;
}


//tisk matice
//m odkaz na matici
void printMatrix(Tmatrix* m)
{
    char charCode;
    
    printf("%u %u\n", m->rows, m->cols);
    
    for(unsigned int i = 0; i < m->rows; ++i)
    {
	for(unsigned int j = 0; j < m->cols; ++j)
	{
	    charCode = m->arr[i][j];
	    if(charCode == CH) printf("%2s ", "ch");
	    else if(charCode == bigCH) printf("%2s ", "CH");
	    else printf("%1c ", charCode);
	}	
	printf("\n");
    }
}

//p��prava slova pro hled�n�
//searchWord slovo
//matrix odkaz ma matici
bool findWord(char* searchWord, Tmatrix* matrix)
{
    int chTimes = 0;
    int len = strlen(searchWord);
    int wordFound;
    bool foundLastTime = false;
    
    for(int i = 0; i < len; ++i)
    {
	if(foundLastTime) 
	{
	    foundLastTime = false;
	    continue;
	}
	else if(i < len-1 && lowerChar(searchWord[i]) == 'c' && lowerChar(searchWord[i+1]) == 'h')
	{
	    searchWord[i-chTimes] = CH;
	    ++chTimes;
	    foundLastTime = true;
	}
	else 
	{
	    searchWord[i-chTimes] = lowerChar(searchWord[i]);
	}
    }
    len -= chTimes;
       
    //vyhled�n� slova
    wordFound = matrixFind(matrix, searchWord, len);
    return wordFound;
}


//�e�en� osmism�rky
//m odkaz na matici
//wordFile n�zev souboru se slovy
int solve(Tmatrix* m, char* wordsFile)
{
    FILE* f = fopen(wordsFile, "r");
    bool found;
    char word[100];
    
    if(f == NULL) return EFOPENFAIL;
    
    while(fscanf(f, "%100s", word) != EOF)
    {
	found = findWord(word, m);
	if(!found) return ESOLVEFAIL;
	
    }
    
    return true;
}

//vyps�n� tajenky
//m odkaz na matici
void printResult(Tmatrix* m)
{
    char charCode;
    for(unsigned int i = 0; i < m->cols*m->rows; ++i)
    {
	charCode = m->arr[i/m->cols][i%m->cols];
	if(charCode == CH) printf("ch");
	else if(islower(charCode)) printf("%c", charCode);
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    int param;
    
    if(argc > 1)
    {
	
	param = getParam(argv[1]);
	
	if(argc == 2 && param == HELP)
	{
	    printHelp();
	}
	else if(argc > 2)
	{
	    Tmatrix matrix;
	    int numsRead = matrixFill(argv[2], &matrix);
	    
	    //p�i �ten� nastala chyba
	    if(numsRead == EFOPENFAIL) 
	    {
		printError(EFOPEN);
		return (EXIT_FAILURE);
	    }
	    else if(numsRead == ESTRUCTUREFAIL || (numsRead != (int) matrix.cols* (int) matrix.rows))
	    {
		printError(ESTRUCTURE);
		return (EXIT_FAILURE);
	    }
	    else if(numsRead ==	 EMALLOCFAIL)
	    {
		printError(EMALLOC);
		return (EXIT_FAILURE);
	    }
	    else
	    {	    
		int result;
		
		//v�pis osmism�rky
		if(argc == 3 && param == TEST)
		{
		    printMatrix(&matrix);
		}
		//hled�n� slova
		else if(argc == 3 && param == SEARCH)
		{
		    char* searchWord = argv[1]+strlen(PARAMS[SEARCH])*sizeof(char);
		    
		    if(*searchWord != '\0')
		    {
			result = findWord(searchWord, &matrix);
			if(!result) 
			{
			    printError(EWORDNOTFOUND);
			    return (EXIT_FAILURE);
			}
			else
			{
			    printMatrix(&matrix);
			}
		    }
		    else
		    {
			printError(EPARAM);
			return (EXIT_FAILURE);		    
		    }
		}
		//�e�en� osmism�rky
		else if(argc == 4 && param == SOLVE)
		{
		    result = solve(&matrix, argv[3]);
		    if(result == ESOLVEFAIL)
		    {
			printError(ESOLVE);
			return (EXIT_FAILURE);
		    }
		    else if(result == EFOPENFAIL)
		    {
			printError(EFOPEN);
			return (EXIT_FAILURE);
		    }
		    else
		    {
			printResult(&matrix);
		    }
		}
		else 
		{
		    printError(EPARAM);
		    return (EXIT_FAILURE);
		}
	    }
	    
	    matrixFree(&matrix);
	}
	else
	{
	    printError(EPARAM);
	    return (EXIT_FAILURE);	    
	}
    }
    else
    {
	printError(EPARAM);
	return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
