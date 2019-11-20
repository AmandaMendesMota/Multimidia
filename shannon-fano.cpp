/*
 *  Shannon-Fano Implementa��o
 *  Projeto desenvolvido para a disciplina de EC215 - Multimidia
 *  Alunos: Amanda de C�ssia Mendes Mota - Matr�cula: 1247
 *  		Rafael Maciel dos Reis - Matr�cula: 977
 *	Curso: Engenharia da Computa��o
 *	INATEL - Instituto Nacional de Telecomunica��es
 */

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <string.h>
#include <locale.h>

using namespace std;

struct pno
{
	char caracter;
	float probabilidade;
};

static int pno_comparacao( const void *elemento1, const void *elemento2 )
{
	const pno x = *(pno*)elemento1;
	const pno y = *(pno*)elemento2;
	if( x.probabilidade < y.probabilidade ) return 1; //1 - menor (ordem decrescente)
	else if( x.probabilidade > y.probabilidade ) return -1;
	return 0;
}

class Codificador
{
private:
	int tamanho_tabela; //tamanho da tabela(n�mero de caracteres)
	pno *ptabela; //tabela de probabilidades
	map<char, string> codigos; //palavra-c�digo para cada caracter

public:
	void Codificar( const char *inputFilename, const char *outputFilename, int bin )
	{
		map<char, int> frequencia; //Frequ�ncia para cada caracter do texto de entrada
		int i;

		//Abrindo arquivo para leitura

		FILE *inputFile;
		inputFile = fopen( inputFilename, "r" );
		assert( inputFile );

		//Contando caracteres

		char caracter;
		unsigned total = 0;
		while( fscanf( inputFile, "%c", &caracter ) != EOF )
		{
			frequencia[caracter]++;
			total++;
		}
		tamanho_tabela = (int)frequencia.size();

		//Construindo de forma decrescente a tabela de frequ�ncia

		ptabela = new pno[tamanho_tabela];
		assert( ptabela );
		float ftot = float(total);
		map<char, int>::iterator fi;
		for( fi = frequencia.begin(), i = 0; fi != frequencia.end(); ++fi, ++i )
		{
			ptabela[i].caracter = (*fi).first;
			ptabela[i].probabilidade = float((*fi).second) / ftot;
		}
		qsort( ptabela, tamanho_tabela, sizeof(pno), pno_comparacao );

		//Codificando

		CodShannon( 0, tamanho_tabela - 1 );

		//Abrindo arquivo de sa�da para escrita

		FILE *outputFile;
		outputFile = fopen( outputFilename, "wb" );
		assert( outputFile );


		if(bin)
		{
			printf( "%i""\n", tamanho_tabela );

			for( i = 0; i < tamanho_tabela; i++ )
			{
				printf("%c\t%f\t%s""\n", ptabela[i].caracter, ptabela[i].probabilidade, codigos[ptabela[i].caracter].c_str() );

			}
		}
		else
		{
			printf( "%i""\n", tamanho_tabela );
			fprintf( outputFile, "%i""\n", tamanho_tabela );

			for( i = 0; i < tamanho_tabela; i++ )
			{
				printf("%c\t%f\t%s""\n", ptabela[i].caracter, ptabela[i].probabilidade, codigos[ptabela[i].caracter].c_str() );
				fprintf(outputFile, "%c\t%f\t%s""\n", ptabela[i].caracter, ptabela[i].probabilidade, codigos[ptabela[i].caracter].c_str() );
			}
		}

		//Sa�da de texto codificado

		fseek( inputFile, SEEK_SET, 0 );
		printf("\n");
		fprintf(outputFile, "\n");
		while( fscanf( inputFile, "%c", &caracter ) != EOF )
		{
			printf("%s", codigos[caracter].c_str());
			fprintf(outputFile, "%s", codigos[caracter].c_str());
		}
		printf("\n");

		//Limpando

		codigos.clear();
		delete[] ptabela;

		//Fechando arquivos

		fclose( outputFile );
		fclose( inputFile );
	}

	void Decodificador( const char *inputFilename, const char *outputFilename )
	{
		//Abrindo arquivo para leitura

		FILE *inputFile;
		inputFile = fopen( inputFilename, "r" );
		assert( inputFile );

		//Carregando c�digos

		fscanf( inputFile, "%i", &tamanho_tabela );
		char caracter, code[128];
		float probabilidade;
		int i;
		fgetc( inputFile ); //pular linha no final
		for( i = 0; i < tamanho_tabela; i++ )
		{
			caracter = fgetc(inputFile);
			fscanf( inputFile, "%f %s", &probabilidade, code );
			codigos[caracter] = code;
			fgetc(inputFile); //pular linha no final
		}
		fgetc(inputFile); //pular linha no final

		//Abrindo arquivo de sa�da

		FILE *outputFile;
		outputFile = fopen( outputFilename, "w" );
		assert( outputFile );

		//Decodificando e escrevendo no arquivo

		string accum = "";
		map<char, string>::iterator ci;
		while((caracter = fgetc(inputFile)) != EOF)
		{
			accum += caracter;
			for( ci = codigos.begin(); ci != codigos.end(); ++ci )
				if( !strcmp( (*ci).second.c_str(), accum.c_str() ) )
				{
					accum = "";
					printf( "%c", (*ci).first );
					fprintf( outputFile, "%c", (*ci).first );
				}
		}
		printf("\n");

		//Fechando arquivos

		fclose( outputFile );
		fclose( inputFile );
	}

private:
	void CodShannon( int li, int ri )
	{
		int i, isp;
		float probabilidade;
		float probabilidade_total;
		float probabilidade_parcial;

		if( li == ri )
		{
			return;
		}
		else if( ri - li == 1 )
		{
			//Se o intervalo � composto por 2 elementos

			codigos[ptabela[li].caracter] += '0';
			codigos[ptabela[ri].caracter] += '1';
		}
		else
		{
			//C�lculo da soma de probabilidades em um intervalo espec�fico

			probabilidade_total = 0;
			for( i = li; i <= ri; ++i )
			{
				probabilidade_total += ptabela[i].probabilidade;
			}

			//Procurando o meio
			probabilidade = 0;
			isp = -1; //�ndice de posi��o dividida
			probabilidade_parcial = probabilidade_total * 0.5f;
			for( i = li; i <= ri; ++i )
			{
				probabilidade += ptabela[i].probabilidade;
				if(probabilidade <= probabilidade_parcial)
				{
					codigos[ptabela[i].caracter] += '0';
				}
				else
				{
					codigos[ptabela[i].caracter] += '1';
					if( isp < 0 ) isp = i;
				}
			}

			if( isp < 0 ) isp = li + 1;

			//Pr�ximos passos
			CodShannon( li, isp - 1 );
			CodShannon( isp, ri );
		}
	}
};


void inpubin(const char *inputFilename, const char *outputFilename)
{

	//Abrindo arquivo para leitura entrada

	FILE *inputFile;
	inputFile = fopen( inputFilename, "r" );
	assert( inputFile );

	//Abrindo arquivo para sa�da

	FILE *outputFile;
	outputFile = fopen( outputFilename, "wb" );
	assert( outputFile );

	//Criando o bin
	char caracter;
	char caracter2[8];

	for(int i = 0; i < 8; i++) caracter2[i] = 0;

	int bin;

	fseek( inputFile, SEEK_SET, 0 );

	while( fscanf( inputFile, "%c", &caracter ) != EOF )
	{
		bin = caracter;

		int a = 0;

		while (bin > 0)
		{
			(bin % 2) ? caracter2[a] = 1 : caracter2[a] = 0;
			bin /= 2;
			a++;
		}

		for(a = 7; a >= 0; a--)
		{
			fprintf(outputFile, "%d", caracter2[a]);
		}
	}


	//Fechando arquivos

	fclose( outputFile );
	fclose( inputFile );

}

int como_usar()
{
	printf("Compress�o/Descompress�o Shannon-Fano""\n");
	printf("\n");
	printf("M�todos de Uso: ""\n");
	printf("Compress�o: ""\n");
	printf(" ./shannon-fano nomedoarquivo.txt""\n");
	printf("Descompress�o: ""\n");
	printf(" ./shannon -d codificado.txt""\n");
	printf("\n");
	exit(0);
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "Portuguese");//habilita a acentua��o para o portugu�s

	int i = 1;
	int dFlag = 0;
	char inputFilename[128];
	char outputFilename[128];

	printf("\n");

	if(i == argc)
	{
		como_usar();
	}

	if(strcmp(argv[i], "-d") == 0)
	{
		dFlag = 1;
		i++;
		if(i == argc) como_usar();
	}

	strcpy(inputFilename, argv[i]);
	i++;

	if(i < argc)
	{
		strcpy(outputFilename, argv[i]);
	}
	else
	{
		if(dFlag)
		{
			strcpy(outputFilename, "decodificado.txt");
		}
		else
		{
			strcpy(outputFilename, "codificado.txt");
		}
	}

	//Chamando codificador e decodificador

	Codificador *codificador;
	codificador = new Codificador;
	assert(codificador);
	if(!dFlag)
	{
		codificador->Codificar( inputFilename, outputFilename, 0 );
	}
	else
	{
		codificador->Decodificador( inputFilename, outputFilename );
	}

	codificador->Codificar( inputFilename, "codificado_bin.txt", 1 );

	printf("\n");

	inpubin( inputFilename, "shannon-fano_bin.txt" );

	delete codificador;

	printf("\n");

	return 0;
}
