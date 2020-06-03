#include <bits/stdc++.h>
#include "FileSystem.h"

int main(int argc, char* argv[])
{
	printf("1 - Abrir arquivo de imagem existente\n2 - Criar novo arquivo\n");
	int opt;
	char *filename = (char*)malloc(1000);
	
	
	
	scanf("%d", &opt);
	
	printf("Nome do arquivo: ");
		
	scanf("%s",filename);
	
	FileSystem fs(filename);
	
	if(opt==1)
	{
		fs.open();

		printf("\n");
	}
	else
	{
		int tam,sec;
		printf("Tamanho da particao (em setores): ");
		scanf("%d",&tam);
		printf("Tamanho do setor: ");
		scanf("%d",&sec);
		
		fs.formatar(tam,sec);
	}
	
	printf("Operacoes:\n    1 - listar o conteúdo do diretorio\n    2 - ir para um diretorio\n    3 - criar um novo diretorio\n    4 - formatar\n    5 - copiar um arquivo do disco para o diretorio atual\n    6 - copiar um arquivo do diretorio atual para o disco\n    7 - excluir um arquivo/diretório\n    8 - ajuda\n    9 - sair\n\n");
		
	
	//opções
	while(opt!=9)
	{
		scanf("%d", &opt);
		if(opt == 1)
		{
			fs.listarDir();
			printf("\n");
		}
		else if(opt == 2)
		{
			char *dirName = (char*)malloc(1000);
			printf("Diretorio: ");
			scanf("%s", dirName);
			
			
			fs.goToDir(dirName);
			
			printf("\n");
		}
		else if(opt == 3)
		{
			printf("Nome do diretorio: ");
			char *dirName = (char*)malloc(1000);
			scanf("%s", dirName);
			fs.criarDir(dirName);
			

			printf("\n");
		}
		else if(opt == 4)
		{
			int tam,sec;
			printf("Tamanho da particao (em setores): ");
			scanf("%d",&tam);
			printf("Tamanho do setor: ");
			scanf("%d",&sec);
			
			fs.formatar(tam,sec);
		}
		else if(opt == 5)
		{
			printf("Nome do arquivo a copiar: ");
			char *dirName = (char*)malloc(1000);
			scanf("%s", dirName);
			
			fs.copiaIn(dirName);
		}
		else if(opt == 6)
		{
			printf("Nome do arquivo a copiar: ");
			char *dirName = (char*)malloc(1000);
			scanf("%s", dirName);
			
			fs.copiaOut(dirName);
		}
		else if(opt == 7)
		{
			printf("Nome do arquivo ou diretório a excluir: ");
			char *dirName = (char*)malloc(1000);
			scanf("%s", dirName);

			fs.deletar(dirName);
			
		}
		else if(opt == 8)
		{
			printf("\n\nOperacoes:\n    1 - listar o conteúdo do diretorio\n    2 - ir para um diretorio\n    3 - criar um novo diretorio\n    4 - formatar\n    5 - copiar um arquivo do disco para o diretorio atual\n    6 - copiar um arquivo do diretorio atual para o disco\n    7 - excluir um arquivo/diretório\n    8 - ajuda\n    9 - sair\n\n");
		}
	}
	

    return 0;

}
