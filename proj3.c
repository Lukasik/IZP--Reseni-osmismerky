#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

//definice struktury pro uchování matice
typedef struct
{
    unsigned int cols;
    unsigned int rows;
    char **arr;
} Tmatrix;

//definice typu pro ukládání souøadnic pole
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

//definice hodnot chybových kódù
enum ERRORCODES
{
    EPARAM,
    EFOPEN,
    ESTRUCTURE,
    EWORDNOTFOUND,
    ESOLVE,
    EMALLOC,
};

//definice hodnot parametrù
enum PARAMCODES 
{
    TEST,
    SEARCH,
    SOLVE,
    HELP,
    NOPARAM,
};

//definice parametrù
const char* PARAMS[] =
{
    [TEST] = "--test",
    [SEARCH] = "--search=",
    [SOLVE] = "--solve",
    [HELP] = "-h",
};

//definice chybových hlá¹ení
const char* ERRORS[] = 
{
    [EPARAM] = "©patné parametry",
    [EFOPEN] = "Soubor se nepodaøilo otevøít",
    [ESTRUCTURE] = "Soubor má ¹patný formát",
    [EWORDNOTFOUND] = "Slovo nenalezeno",
    [ESOLVE] = "Nepodaøilo se vylu¹tit tajenku, slovo nenalezeno",
    [EMALLOC] = "Chyba pøi alokaci pamìti",
};

const char CH = '#'; //hodnota písmena ch v matici
const char bigCH = '$'; //velké ch
const int ESTRUCTUREFAIL = -1;
const int EFOPENFAIL = -2;
const int ESOLVEFAIL = -3;
const int EMALLOCFAIL = -4;
const int waysCount = 8;

//výpis nápovìdy
void printHelp(void)
{
    printf("%s",
	   	"NÁZEV\n"
		"	proj3\n"
		"AUTOR\n"
		"	Luká¹ Vokráèko\n"
		"POU®ITÍ\n"
		"	proj3 --test|--search=slovo|--solve|--help [zdrojový soubor] [soubor se slovy]\n"
		"PARAMETRY\n"
		"	-h - tiskne nápovìdu\n"
		"	--test - otestuje a vytiskne osmismìrku\n"
		"	--search=slovo - najde a zvýrazní hledané slovo\n"
		"	--solve - vyøe¹í osmismìrku a vytiskne výsledek\n"
	   );    
}

//výpis chyb
//errType index chybové zprávy
void printError(int errType)
{
    fprintf(stderr, "%s\n", ERRORS[errType]);
}

//hledání slova s jeho vyznaèením
//m odkaz na matici
//searchWord pole kódù pro písmena
//searchWordSize velikost pole
int matrixFind(Tmatrix* m, char* searchWord, int searchWordSize)
{
    //deklarace v¹ech smìrù
    Tcoord directions[8] = {{-1,0}, {-1,+1}, {0,+1}, {+1,+1}, {+1,0}, {+1,-1}, {0,-1}, {-1,-1}};
    Tcoord* singleCoords = malloc(sizeof(Tcoord)*searchWordSize);
    int founds = 0;
    char charCode;
    
    //hledání prvního znaku
    for(unsigned i = 0; i < m->cols*m->rows; ++i)
    {
	//výpoèet indexù
	unsigned int row = i/m->cols;
	unsigned int col = i%m->cols;

	charCode = lowerChar(searchWord[0]);
	//první znak nalezen
	if(charCode == lowerChar(m->arr[row][col]))
	{
	    //ulo¾ím jeho pozici abych ho poté mohl zvýraznit
	    singleCoords[0].row = row;
	    singleCoords[0].col = col;
	    
	    unsigned int newRow = row;
	    unsigned int newCol = col;
	    
	    //zkou¹ím smìry
	    for(int j = 0; j < waysCount; ++j)
	    {
		newRow = row;
		newCol = col;
		//kontrola znak po znaku
		for(int k = 1; k < searchWordSize; ++k)
		{
		    //posunu se o jeden znaka ve smìru
		    newRow = newRow + directions[j].row;
		    newCol = newCol + directions[j].col;
		    
		    //kontrola, jestli se nové souøadnice nachází v matici
		    if(newCol < m->cols && newRow < m->rows)
		    {
			charCode = lowerChar(searchWord[k]);
			//odpovídá dal¹í písmeno?
			if(charCode == lowerChar(m->arr[newRow][newCol]))
			{
			    //ulo¾ jeho souøadnice pro pozdìj¹í zvýraznìní
			    singleCoords[k].row = newRow;
			    singleCoords[k].col = newCol;
			    
			    //bylo to poslední písmeno?
			    if(k == searchWordSize-1) 
			    {
				++founds;
				highlighWord(m, singleCoords, searchWordSize);
			    }
			    //a pokraèuj hledáním dal¹ího
			    else continue;
			}
			else break;
		    }
		    //pozice je mimo matici nebo se znaky neshodují, není tøeba zku¹et dal¹í
		    break;
		}
	    }
	}
    }
        
    return founds;
}

//pøevedení znakù na velká písmena
//m dokaz na matici
//c pole indexù, kde majíbýt velká písmena
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

//alokace pamìti pro matici
//rows poèet øádkù
//cols poèet sloupcù
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

//uvolnìní pamìti vyhrazené pro matici
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

//je to písmeno nebo CH?
//loadedStr - naètené písmeno
//charCode adresa pro ulo¾ení znaku
bool isValidChar(char* loadedStr, char* charCode)
{
    int len = strlen(loadedStr);
    
    if(len == 1 && isalpha(loadedStr[0]))
    {
	*charCode = (char) tolower(loadedStr[0]);
    }
    //naèteny 2 znaky
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

//naplnìní matice ze souboru
//filename název souboru
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
    //nepodaøilo se otevøít soubor
    if(f == NULL) return EFOPENFAIL;

    //naètení rozmìrù
    code = fscanf(f, "%d", &rows);
    code += fscanf(f, "%d", &cols);
    //nepovedlo se naèíst parametry
    if(code != 2 || cols <= 0 || rows <= 0) return ESTRUCTUREFAIL;

    maxNums = cols*rows;
    *m = matrixAlloc(rows, cols);
    if(m->arr == NULL) return EMALLOCFAIL;

    //ètení znakù
    while(((code = fscanf(f, "%2s", letter)) != EOF) && readNums != maxNums)
    {
	if((code == 1) && isValidChar(letter, &charCode))
	{
	    m->arr[readNums / m->cols][readNums % m->cols] = charCode;
	}
	//¹patný symbol
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

//naèítání parametrù
//param textový parametr
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

//zmìn¹ení znaku
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

//zvìt¹ení znaku
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

//pøíprava slova pro hledání
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
       
    //vyhledání slova
    wordFound = matrixFind(matrix, searchWord, len);
    return wordFound;
}


//øe¹ení osmismìrky
//m odkaz na matici
//wordFile název souboru se slovy
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

//vypsání tajenky
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
	    
	    //pøi ètení nastala chyba
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
		
		//výpis osmismìrky
		if(argc == 3 && param == TEST)
		{
		    printMatrix(&matrix);
		}
		//hledání slova
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
		//øe¹ení osmismìrky
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
