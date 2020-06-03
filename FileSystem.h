#include <bits/stdc++.h>

#define entrada 4
#define nTables 2
#define entradaArquivo 32

using namespace std;

//struct que define o formato do boot record.
struct bootSector 
{
	int8_t espaco_reservado[8];
	int16_t tamanhoSetor;
	int8_t setoresPorCluster;
	int8_t numeroTabelas;
	int clustersPorTabela;
	
};typedef struct bootSector bootSector;

//struct que define o formato de uma entrada de arquivo/diretório.
struct fileEntry 
{
	int8_t atributosEntrada;
	int8_t espaco_reservado[3];
	unsigned int creationTime;
	unsigned int lastModTime;
	unsigned int lastAccessTime;
	unsigned int firstFatCluster;
	
	int fileSize1, fileSize2;
	
	int espaco_reservado2;
	
};typedef struct fileEntry fileEntry;

//struct que define o formato de uma entrada de simple file name.
struct simpleFileName 
{
	int8_t atributos;
	int8_t bytesUsados;
	char nome[30]; 
	
};typedef struct simpleFileName simpleFileName;


class FileSystem
{
	private: bootSector *BS;
	private: int tamCluster;
	private: int entradasPorCluster;
	private: int qtdClustersUse;
	private: int tamImg;
	private: int ttlClusters;
	private: char* filename;
	
	private: int dirAtual = 0;
	
	FILE *img;
	
	/*Construtor da classe FyleSistem, que gerencia o sistema de arquivos.
	 * 
	 * fname - nome do arquivo de imagem.
	 */
	public: FileSystem(char *fname)
	{
		filename = fname;
	}

	//chama a função de exclusão.
	public: void deletar(char *name)
	{
		int tamPath = strlen(name);
		bool flag=false;
		char *aux = (char*)malloc(150);

		int j = 0;
		for(int i=0;i<tamPath;i++)
		{
			if(name[i]!='/')
			{
				aux[j]=name[i];
				j++;
				
			}
			else
			{
				flag=true;
				aux[j]=0;
				j=0;
			}
			
		}
		aux[j]=0;
		j=0;

		if(flag)
		{
			for(int i=tamPath-1;i>=0;i--)
			{
				if(name[i]=='/')
				{
					name[i]=0;
					break;
				}

				name[i]=0;
			}
			excluir(findDir(name,false),aux);
		}
		else
		{
			excluir(dirAtual,aux);
		}
		
		


		
	}

	/*leva o cursor do sistema até um novo diretório.
	 *
	 * dirName - nome do diretório.
	 */
	public: void goToDir(char *dirName)
	{
		int d = findDir(dirName,false);	

		if(d!=0)
		{
			dirAtual = 	d;
		}
	}
	
	//chama a função de listar os arquivos.
	public: void listarDir()
	{
		listarArquivos(dirAtual);
	}
	
	//chama a função de encontrar diretórios (findDir), porém com a flag de criação ativa.
	public: void criarDir(char *dirName)
	{
		findDir(dirName,true);
	}
	
	//chama a função de cópia para dentro.
	public: void copiaIn(char *dirName)
	{
		copiarPraDentro(dirAtual, dirName);
	}
	
	//chama a função de cópia para fora.
	public: void copiaOut(char *dirName)
	{
		copiarPraFora(dirAtual, dirName);
	}
	
	/*Encontra o primeiro cluster de um diretório com base em um caminho dado (Exemplo "a/b/c/d", encontra o primeiro cluster de "d")
	 *
	 * path - o caminho do diretório.
	 *
	 * create - uma flag que define se os diretórios inexistentes serão criados.
	 * 
	 * retorno - o primeiro cluster do diretório.
	 */
	private: int findDir(char *path, bool create)
	{
		int tamPath = strlen(path);

		int dir=dirAtual;

		list<char*> pathDirs;
		char **aux = (char**)malloc(sizeof(*aux)*50);
		for(int i = 0;i<50;i++)
		{
			aux[i] = (char*)malloc(150);
		}


		int j = 0;
		int c = 0;
		for(int i=0;i<tamPath;i++)
		{
			if(path[i]!='/')
			{
				aux[c][j]=path[i];
				j++;
			}
			else
			{
				aux[c][j]=0;
				j=0;
				c++;
				pathDirs.push_back(aux[c-1]);	
			}
			
		}
		aux[c][j]=0;
		j=0;
		pathDirs.push_back(aux[c]);

		for(auto i:pathDirs)
		{
			int entryAddr = findEntry(dir,i);
			if(entryAddr!=0)
			{
				img = fopen(filename,"r+");
				fseek(img,entryAddr+16,SEEK_SET);
				fread(&dir,4,1,img);
				fclose(img);
			}
			else if(create)
			{
				criarDiretorio(dir,i);
				entryAddr = findEntry(dir,i);
				img = fopen(filename,"r+");
				fseek(img,entryAddr+16,SEEK_SET);
				fread(&dir,4,1,img);
				fclose(img);
			}
			else
			{
				printf("Diretório não existe.\n");
				return 0;
			}
			
			
		}
		return dir;
	}

	/*separa os quatro bytes menos significativos de um número de oito bytes. (usado para corrigir um problema onde um número de 8bits deve de ser representado por dois números de 4bits)
	 * 
	 * a: número de oito bytes;
	 * 
	 * retorno:os quatro bytes menos significativos de a.
	 */
	private: int getLow32(long a)
	{
		return a%4294967296;
	}

	/*separa os quatro bytes mais significativos de um número de oito bytes. (usado para corrigir um problema onde um número de 8bits deve de ser representado por dois números de 4bits)
	 * 
	 * a: número de oito bytes;
	 * 
	 * retorno:os quatro bytes mais significativos de a.
	 */
	private: int getHigh32(long a)
	{
		return a/4294967296;
	}

	/*organiza as informações derivadas do Boot Sector
	 */
	private: void setBSInfo()
	{
		img = fopen(filename, "r+");//abre o arquivo de imagem em modo que permite leitura e edição.
		fseek(img,0,SEEK_SET); //move o cursor do arquivo para o início.
		fread(BS, sizeof(bootSector), 1, img);
		
		tamCluster = BS->setoresPorCluster*BS->tamanhoSetor; //calcula o tamanho do cluster.
		
		entradasPorCluster = tamCluster/entrada; //calcula a quantidade de entradas (FAT) em cada cluster.
		
		ttlClusters = tamImg/tamCluster;//calcula a quantidade total de clusters no sistema.
		
		qtdClustersUse = ttlClusters-((nTables*BS->clustersPorTabela)+1);//calcula a quantidade de clusters não usados pelo BS ou pelas FATs.
		
		fclose(img);
	}
	
	/*escreve um valor em um determinado cluster em cada uma das tabelas de alocação de arquivos.
	 * 
	 * cluster: o número do cluster a ser escrito
	 * 
	 * val: o valor a ser escrito
	 */
	private: void writeFAT(int cluster, int val) 
	{
		if(cluster<qtdClustersUse)
		{
			img = fopen(filename, "r+");

			int addr;
			
			for(int i = 0;i<BS->numeroTabelas;i++)
			{
				addr = tamCluster//boot record
						+ (i*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //tabelas anteriores
						+ (cluster*4); //deslocamento dentro da tabela atual
				
				fseek(img,addr,SEEK_SET);
				fwrite(&val, 4, 1, img);
			}
			
			fclose(img);
		}	
		else
		{
			printf("Sem espaço na FAT\n");
		}
		
	}
	
	/*lê o valor de um determinado cluster, verificando a integridade do mesmo através de todas as FATs.
	 * 
	 * cluster: cluster a ser lido.
	 * 
	 * retorno: o valor do cluster lido na FAT, retorna 0 se houver algum erro.
	 */
	private: int readFAT(int cluster)
	{
		if(cluster>qtdClustersUse)
		{
			printf("Impossivel ler esse cluster na FAT porque não está dentro da faixa de clusters usáveis.\n");
			return 0;
		}
		img = fopen(filename,"r");

		int addr;
		
		int fatEntry;
		int aux;
		
		for(int i = 0;i<BS->numeroTabelas;i++)
		{
			addr = tamCluster//boot record
					+ (i*(BS->clustersPorTabela*(BS->tamanhoSetor*BS	->setoresPorCluster))) //tabelas anteriores
					+ (cluster*4); //deslocamento dentro da tabela atual
			
			fseek(img,addr,SEEK_SET);
			fread(&aux,4,1,img);
			if(i!=0 && aux!=fatEntry)
			{
				printf("Erro na FAT no cluster %d\n", cluster);
				return 0;
			}
			fatEntry = aux;
		}
		
		fclose(img);
		
		return fatEntry;
		
	}
	
	/*organiza as informações ao abrir um arquivo de imagem.
	 */
	public: void open()
	{
		BS = (bootSector*)malloc(sizeof(bootSector));
		img = fopen(filename,"r+");
		fseek(img,0,SEEK_SET);
		fread(BS,sizeof(bootSector),1,img);
		
		fseek(img,0,SEEK_END);
		tamImg=ftell(img);
		fclose(img);
		setBSInfo();
	}

	/*formata o sistema de arquivos.
	 * 
	 * nSecs: número de setores no sistema.
	 * 
	 * secSize: tamanho de cada setor.
	 */
	public: void formatar(int nSecs, int16_t secSize)
	{
			//informações do boot sector
			tamImg = nSecs*secSize;
			int8_t secPorCluster = pow(2,floor((tamImg)/(pow(2,32))));
			int tamCluster = secPorCluster * secSize;
			int ttlClusters = (tamImg/tamCluster);
			int ttlClustersDados = ttlClusters-1;
			int entradasPorCluster = (tamCluster/entrada);
			int clustersPorTabela =  ceil((double)(ttlClustersDados)/(double)(entradasPorCluster));

			BS = (bootSector*)malloc(sizeof(bootSector));
			memset(BS->espaco_reservado,0,8);
			BS->tamanhoSetor = secSize;
			BS->setoresPorCluster = secPorCluster;
			BS->numeroTabelas = nTables;
			BS->clustersPorTabela = clustersPorTabela;
			
			
			
			//-------------------------------------------------------------------------------------------------------------------------------------------
			
			//criação da imagem
			img = fopen(filename,"w"); //abre o arquivo de imagem em modo de escrita, nesse modo, é aberto um arquivo totalmente novo, sobrescrevendo qualquer arquivo anterior.
			
			int8_t temp = 0;
			
			for(int i = 0;i<tamImg;i++)
			{
				fwrite(&temp, 1, 1, img); //escreve o vetor temp (que é 0) em cada posição da imagem, com intuito de zerar todo o sistema de arquivos.
			}

			fclose(img); 
			//-------------------------------------------------------------------------------------------------------------------------------------------
			
			//escrevendo informações do boot sector
			
			img = fopen(filename,"r+");
			fwrite(BS,sizeof(bootSector),1,img); //escreve o boot sector na imagem.
			fclose(img);
			
			setBSInfo();
			
			//-------------------------------------------------------------------------------------------------------------------------------------------
			
			//root
			writeFAT(0,0xFFFFFFFF); //escreve o endereçamento da root na FAT, que, inicialmente, é endereçada por apenas um cluster.
			
			int rootAddr = tamCluster*(1 + (nTables*clustersPorTabela)); //endereço da root
			
			
			//entradas padrão
				fileEntry *ponto = (fileEntry*)malloc(sizeof(fileEntry));
				fileEntry *pontoPonto = (fileEntry*)malloc(sizeof(fileEntry));
				
				
				//setando espaços reservaods
				memset(ponto->espaco_reservado,0,3);
				memset(pontoPonto->espaco_reservado,0,3);
				ponto->espaco_reservado2=0;
				pontoPonto->espaco_reservado2=0;
				//------------------------------------------------------------------
				
				ponto->atributosEntrada = 0x10; //entrada do tipo arquivo
				ponto->creationTime = (unsigned)time(NULL);
				ponto->lastModTime = (unsigned)time(NULL);
				ponto->lastAccessTime = (unsigned)time(NULL);
				ponto->firstFatCluster = 0;
				ponto->fileSize1 = 0;
				ponto->fileSize2 = 0;
				
				pontoPonto->atributosEntrada = 0x10; //entrada do tipo arquivo
				pontoPonto->creationTime = (unsigned)time(NULL);
				pontoPonto->lastModTime = (unsigned)time(NULL);
				pontoPonto->lastAccessTime = (unsigned)time(NULL);
				pontoPonto->firstFatCluster = 0;
				pontoPonto->fileSize1 = 0;
				pontoPonto->fileSize2 = 0;
				
				
				//simple file names
				simpleFileName *pontoSFN = (simpleFileName*)malloc(sizeof(simpleFileName));
				simpleFileName *pontoPontoSFN = (simpleFileName*)malloc(sizeof(simpleFileName));
				
				pontoSFN->atributos=0x30;
				pontoSFN->bytesUsados=1;
				strcpy(pontoSFN->nome,".");
				
				pontoPontoSFN->atributos=0x30;
				pontoPontoSFN->bytesUsados=2;
				strcpy(pontoPontoSFN->nome,"..");
				//----------------------------------------------------------------------
				
			//-----------------------------------------------------------------------------------------------------
			
			img = fopen(filename,"r+");
			fseek(img, rootAddr, SEEK_SET);	
			fwrite(ponto, sizeof(fileEntry), 1, img);
			fwrite(pontoSFN, sizeof(simpleFileName), 1, img);
			fwrite(pontoPonto, sizeof(fileEntry), 1, img);
			fwrite(pontoPontoSFN, sizeof(simpleFileName), 1, img);

			
			fclose(img);
			
			//dirAtual =0;
			//-------------------------------------------------------------------------------------------------------------------------------------------		
	}
	
	/*encontra um cluster não alocado na FAT.
	 * 
	 * retorno: o número do cluster livre, caso não haja nenhum cluster livre, retorna 0;
	 */
	private: int findFreeCluster()
	{
		img = fopen(filename,"r+");

		for(int i = 0; i<qtdClustersUse;i++)
		{
			if(readFAT(i)==0)
			{
				return i;
			}
		}
		printf("Nenhum cluster livre encontrado\n");
		return 0;//sem espaço
	}

	/*lê uma sequência de simple file names.
	 * 
	 * addr: endereço do início da sequência.
	 * 
	 * fimAddr: ponteiro para armazenar o endereço final da sequência.
	 * 
	 * retorno: o nome lido.
	 */
	private: char *readFileName(int addr, int *fimAddr)
	{
			img = fopen(filename, "r");
			list<char*> nameShards; //Cada pedaço do nome.
			
			fseek(img,addr,SEEK_SET);
			
			simpleFileName *sfn = (simpleFileName*)malloc(sizeof(simpleFileName));
			
			fread(sfn,sizeof(simpleFileName),1,img);
			char *aux;
			int nameSize=0;
			while(sfn->bytesUsados==0)
			{
					aux= (char*)malloc(sizeof(char)*30);
					strcpy(aux, sfn->nome);

					nameShards.push_back(aux);
					nameSize+=30;
					fread(sfn,sizeof(simpleFileName),1,img);
			}
			aux= (char*)malloc(sizeof(char)*(sfn->bytesUsados+1));
			strcpy(aux, sfn->nome);

			nameShards.push_back(aux);
			nameSize+=sfn->bytesUsados;
			
			char *name = (char*)malloc(nameSize);
			
			int count = 0;
			for(auto i:nameShards)
			{
				if(i != nameShards.back())
				{
					for(int j=0;j<30;j++)
					{
						name[j+(count*30)] = i[j];
					}
				}
				else
				{
					int j;
					for(j=0;j<sfn->bytesUsados;j++)
					{
						name[j+(count*30)] = i[j];
					}
					name[j+(count*30)] = 0;
				}
				
				count++;
			}
			
			*fimAddr = (addr+ (int)((ceil((double)nameSize/30.0))*32));
			
			return name;
	}

	/*exibe o nome de todos os arquivos contidos em um determinado diretório.
	 * 
	 * firstCluster: primeiro cluster do diretório.
	 */
	private: void listarArquivos(int firstCluster)
	{
		printf("Conteúdo do diretorio:\n");

		int aux = firstCluster;
		unsigned int aux2;
		list<int> dirClusters;
		
		do
		{
			
			aux2 = readFAT(aux);
			dirClusters.push_back(aux);
			aux = aux2;
			
		}while(aux2!=0xFFFFFFFF);
		
		int addrCluster;
		
		for(auto i:dirClusters)
		{
			int aux;
			
			addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
					+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
					+ ((BS->tamanhoSetor*BS->setoresPorCluster)*i); //localização do cluster na area de dados
			
			
			
			int entryAddr = addrCluster;

			for(int j=0;j<tamCluster;j+=32)
			{	
				img = fopen(filename, "r");
				fseek(img,addrCluster+j,SEEK_SET);
			
				int8_t entryAttr;
				fread(&entryAttr,1,1,img);
				fclose(img);

				if(entryAttr==0x20 || entryAttr==0x10)
				{
				printf("%s\n", readFileName(entryAddr+j+32,&aux));
				}
				
			}
		}
		
	
	
	}

	/*encontra um espaço livre em um cluster para armazenar um entrada de arquivo/diretório.
	 * 
	 * addr: endereço do inicio do cluster.
	 * 
	 * size: tamanho (em bytes) da entrada, considerando as entradas de simple file name.
	 * 
	 * filename: nome do arquivo de sistema.
	 * 
	 * retorno: retorna o endereço que contém o espaço livre dentro do cluster, caso não haja espaço suficiente para a entrada, retorna 0.
	 */
	private: int findFreeSlot(int addr, int size)
	{
		int8_t entryAttr;
		int slotStart=0;
		int slotEnd;
		
		for(int i = 0; i<tamCluster/entradaArquivo;i++)
		{
			img = fopen(filename,"r");
			fseek(img, (addr+(i*entradaArquivo)), SEEK_SET);
			fread(&entryAttr,1,1,img);
			fclose(img);
			
			if(entryAttr==0)//espaço não alocado
			{
				slotStart = addr+(i*entradaArquivo);
				
				
				for(int j = i; j<tamCluster/entradaArquivo;j++)
				{
					img = fopen(filename,"r");
					fseek(img, (addr+(i*entradaArquivo)), SEEK_SET);
					fread(&entryAttr,1,1,img);
					fclose(img);
					
					if(entryAttr!=0)
					{
						slotEnd= (addr+(i*entradaArquivo));
						
						if((slotEnd-slotStart)>=(size))
						{
							return slotStart; //o espaço é suficiente, retorna o endereço inicial;
						}
						break;
					}
					
					i++;
				}
			}
		}
		if((addr+tamCluster)-slotStart>=(size))
		{
			
			return slotStart; //o espaço é suficiente, retorna o endereço inicial (o espaço está depois de todas as outras entradas);
		}
		
		return 0; //não há espaço suficiente no cluster
	}

	/*encontra uma entrada dentro de um diretório através do nome do diretório/arquivo
	 * 
	 * parentFirstCluster: primeiro cluster do diretório que contém as entradas.
	 * 
	 * nome: nome do diretório ou arquivo cuja entrada deseja-se encontrar.
	 * 
	 * filename: nome do arquivo de sistema.
	 * 
	 * retorno: caso encontre a entrada, retorna o seu endereço, caso contrário, retorna 0.
	 */
	private: int findEntry(int parentFirstCluster, char *nome)
	{
		unsigned int aux = parentFirstCluster;
		unsigned int aux2;
		list<int> dirClusters;
		
		do
		{
			aux2 = readFAT(aux);
			dirClusters.push_back(aux);
			aux = aux2;
			
			
		}while(aux2!=0xFFFFFFFF);
		
		
		int addrCluster;
		
		for(auto i:dirClusters)
		{
			
			
			addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
					+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
					+ ((BS->tamanhoSetor*BS->setoresPorCluster)*i); //localização do cluster na area de dados
			
			
			
			int8_t entryAttr;
			
			
			int entryAddr;


			for(int j = 0; j<tamCluster;j+=entradaArquivo)
			{	
				img = fopen(filename, "r");
				
				entryAddr = addrCluster+j;
				fseek(img,entryAddr,SEEK_SET);
				fread(&entryAttr,1,1,img);

				fclose(img);
				
				if(entryAttr==0x10 || entryAttr==0x20)
				{
					

					int curEntryAddr = entryAddr;
					char *auxNome = readFileName(entryAddr+32,&entryAddr);

					

					if(strcmp(auxNome,nome)==0)
					{
						return curEntryAddr;//encontrou a entrada;
					}

				}

			}

		}
		
		return 0; //nao encontrou a entrada
	}

	/*encontra uma entrada dentro de um diretório através do primerio cluster do diretório/arquivo
	 * 
	 * parentFirstCluster: primeiro cluster do diretório que contém as entradas.
	 * 
	 * firstCluster: primeiro cluster do diretório ou arquivo cuja entrada deseja-se encontrar.
	 * 
	 * filename: nome do arquivo de sistema.
	 * 
	 * retorno: caso encontre a entrada, retorna o seu endereço, caso contrário, retorna 0.
	 */
	private: int findEntry(int parentFirstCluster, int firstCluster)
	{		
		unsigned int aux = parentFirstCluster;
		unsigned int aux2;
		list<int> dirClusters;
		
		do
		{
			aux2 = readFAT(aux);
			dirClusters.push_back(aux);
			aux = aux2;
			
			
		}while(aux2!=0xFFFFFFFF);
		
		int addrCluster;
		
		for(auto i:dirClusters)
		{
			img = fopen(filename, "r");
			
			addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
					+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
					+ ((BS->tamanhoSetor*BS->setoresPorCluster)*i); //localização do cluster na area de dados
			
			int8_t entryAttr;
			
			int entryAddr;

			
			for(int j = 0; j<tamCluster;j+=entradaArquivo)
			{	
				img = fopen(filename, "r");
				
				entryAddr = addrCluster+j;
				fseek(img,entryAddr,SEEK_SET);
				fread(&entryAttr,1,1,img);

				fclose(img);
				
				if(entryAttr==0x10 || entryAttr==0x20)
				{
					
					img = fopen(filename, "r");
					int curEntryAddr = entryAddr;
					fseek(img,entryAddr+16,SEEK_SET);
					int fc;
					fread(&fc,4,1,img);
					fclose(img);
					

					if(firstCluster==fc)
					{
						return curEntryAddr;//encontrou a entrada;
					}

				}

			}
		}
		
		return 0; //nao encontrou a entrada
	}

	/*cria um novo subdiretório dentro do diretório atual.
	 * 
	 * parentFirstCluster: primeiro cluster do diretório atual.
	 * 
	 * name: nome do novo subdiretório.
	 */
	private: void criarDiretorio(int parentFirstCluster, char *name)
	{	
		
		if(findEntry(parentFirstCluster,name))
		{
			printf("Nome já em uso\n");
			return;
		}
		
		int rootAddr = (1+(BS->clustersPorTabela*BS->numeroTabelas))*tamCluster;
		
		int clusterAddr;

		int tamanhoEntradaTotal = ((int)ceil((double)strlen(name)/30.0)+1)*entradaArquivo;
		
		int parentLastCluster = parentFirstCluster;
		int aux = parentFirstCluster;

		int entryAddr;
	
		do
		{
			parentLastCluster=aux;
			clusterAddr = rootAddr + (tamCluster*parentLastCluster);
			entryAddr = findFreeSlot(clusterAddr,tamanhoEntradaTotal);
			if(entryAddr!=0)
			{
				break;
			}
			aux = readFAT(parentLastCluster);

			
		}while(aux!=0xFFFFFFFF);

		
		
		
		
		
		
		if(entryAddr == 0)
		{
			
			int newLastCluster = findFreeCluster();
			if(newLastCluster!=0)
			{
				writeFAT(parentLastCluster,newLastCluster);
				writeFAT(newLastCluster,0xFFFFFFFF);
				parentLastCluster = newLastCluster;
				entryAddr = rootAddr + (tamCluster*parentLastCluster);
			}
			else
			{
				return;
			}
		}
		
		
		fileEntry *entry = (fileEntry*)malloc(sizeof(fileEntry));
		
		simpleFileName *sfn = (simpleFileName*)malloc(sizeof(simpleFileName)*((tamanhoEntradaTotal/entradaArquivo)-1));
		
		int count=0;
		for(int i=0;i<((tamanhoEntradaTotal/entradaArquivo)-1);i++)
		{
			
			if(i!=(((tamanhoEntradaTotal/entradaArquivo)-1))-1)
			{
				sfn[i].atributos = 0x30;
				sfn[i].bytesUsados = 0;
				for(int j = 0;j<30;j++)
				{
					sfn[i].nome[j] = name[j+(i*count*30)];
				}
			}
			else
			{
				sfn[i].atributos = 0x30;
				sfn[i].bytesUsados = strlen(name)-((i*count*30));
				for(int j = 0;j<sfn[i].bytesUsados;j++)
				{
					sfn[i].nome[j] = name[j+(i*count*30)];
				}
			}
			
			count++;
		}
		
		
		
		
		
		
		/*if(entryAddr == 0)
		{
			int newLastCluster = findFreeCluster();
			writeFAT(parentLastCluster,newLastCluster);
			writeFAT(newLastCluster,0xFFFFFFFF);
			entryAddr = rootAddr + (tamCluster*newLastCluster);
		}*/
		
		entry->atributosEntrada = 0x10; //entrada do tipo arquivo
		entry->creationTime = (unsigned)time(NULL);
		entry->lastModTime = (unsigned)time(NULL);
		entry->lastAccessTime = (unsigned)time(NULL);
		entry->firstFatCluster = findFreeCluster();
		entry->fileSize1 = 0;
		entry->fileSize2 = 0;
		writeFAT(entry->firstFatCluster,0xFFFFFFFF);

		img = fopen(filename,"r+");
		fseek(img,entryAddr,SEEK_SET);
		fwrite(entry,sizeof(fileEntry),1,img);
		fwrite(sfn,sizeof(simpleFileName), ((tamanhoEntradaTotal/entradaArquivo)-1), img);
		fclose(img);
		
		//updateDir(parentFirstCluster,true,filename);
		
		img = fopen(filename,"r+");
		
		int parentFirstClusterAddr = rootAddr+(parentFirstCluster*tamCluster);
		fseek(img,parentFirstClusterAddr,SEEK_SET);
		
		fileEntry *parentEntry = (fileEntry*)malloc(sizeof(fileEntry));
		fread(parentEntry,sizeof(fileEntry),1,img);
		
		
		int dirAddr = rootAddr + (entry->firstFatCluster*tamCluster);
		fseek(img,dirAddr,SEEK_SET);
		fwrite(entry,sizeof(fileEntry),1,img);
		
		simpleFileName *dot = (simpleFileName*)malloc(sizeof(simpleFileName));
		dot->atributos=0x30;
		dot->bytesUsados=1;
		strcpy(dot->nome,".");
		
		fwrite(dot,sizeof(simpleFileName),1,img);
		
		fwrite(parentEntry,sizeof(fileEntry),1,img);
		
		simpleFileName *dotDot = (simpleFileName*)malloc(sizeof(simpleFileName));
		dotDot->atributos=0x30;
		dotDot->bytesUsados=2;
		strcpy(dotDot->nome,"..");
		
		fwrite(dotDot,sizeof(simpleFileName),1,img);
		
		fclose(img);
	}
	
	/*copia um arquivo do disco para o diretório atual.
	 * 
	 * parentFirstCluster: primeiro cluster do diretório atual.
	 * 
	 * cpyfilename: nome do arquivo a ser copiado.
	 * 
	 * filename: nome do arquivo de sistema.
	 */
	private: void copiarPraDentro(int parentFirstCluster, char *cpyfilename)
	{	
		
		if(findEntry(parentFirstCluster,cpyfilename))
		{
			printf("Nome já em uso\n");
			return;
		}
		
		FILE *cpy = fopen(cpyfilename,"r+");
		if(cpy==NULL)
		{
			printf("Não foi possivel encontrar o arquivo\n");
			return;
		}
		fclose(cpy);

		int rootAddr = (1+(BS->clustersPorTabela*BS->numeroTabelas))*tamCluster;
		
		int clusterAddr;

		int tamanhoEntradaTotal = ((int)ceil((double)strlen(cpyfilename)/30.0)+1)*entradaArquivo;
		
		int parentLastCluster = parentFirstCluster;
		int aux = parentFirstCluster;

		int entryAddr;
	
		do
		{
			parentLastCluster=aux;
			clusterAddr = rootAddr + (tamCluster*parentLastCluster);
			entryAddr = findFreeSlot(clusterAddr,tamanhoEntradaTotal);
			if(entryAddr!=0)
			{
				break;
			}
			aux = readFAT(parentLastCluster);
			
		}while(aux!=0xFFFFFFFF);

		
		
		
		
		
		
		if(entryAddr == 0)
		{
			
			int newLastCluster = findFreeCluster();
			if(newLastCluster!=0)
			{
				writeFAT(parentLastCluster,newLastCluster);
				writeFAT(newLastCluster,0xFFFFFFFF);
				parentLastCluster = newLastCluster;
				entryAddr = rootAddr + (tamCluster*parentLastCluster);
			}
			else
			{
				return;
			}
		}
		
		fileEntry *entry = (fileEntry*)malloc(sizeof(fileEntry));
		
		simpleFileName *sfn = (simpleFileName*)malloc(sizeof(simpleFileName)*((tamanhoEntradaTotal/entradaArquivo)-1));
		
		int count=0;
		for(int i=0;i<((tamanhoEntradaTotal/entradaArquivo)-1);i++)
		{
			
			if(i!=(((tamanhoEntradaTotal/entradaArquivo)-1))-1)
			{
				sfn[i].atributos = 0x30;
				sfn[i].bytesUsados = 0;
				for(int j = 0;j<30;j++)
				{
					sfn[i].nome[j] = cpyfilename[j+(i*count*30)];
				}
			}
			else
			{
				sfn[i].atributos = 0x30;
				sfn[i].bytesUsados = strlen(cpyfilename)-((i*count*30));
				for(int j = 0;j<sfn[i].bytesUsados;j++)
				{
					sfn[i].nome[j] = cpyfilename[j+(i*count*30)];
				}
			}
			
			count++;
		}
		
		
		
		
		
		
		if(entryAddr == 0)
		{
			int newLastCluster = findFreeCluster();
			writeFAT(parentLastCluster,newLastCluster);
			writeFAT(newLastCluster,0xFFFFFFFF);
			entryAddr = rootAddr + (tamCluster*newLastCluster);
		}
		
		
		cpy = fopen(cpyfilename,"r+");
		fseek(cpy,0,SEEK_END);
		int cpyTam = (int)ftell(cpy);
		int qtdClusters = ceil((double)cpyTam/(double)tamCluster);
		int8_t *cpyVet = (int8_t*)malloc(sizeof(int8_t)*cpyTam);
		fseek(cpy,0,SEEK_SET);
		fread(cpyVet, 1, cpyTam, cpy);

		list<int> cpyClusters;
		fclose(cpy);
		
		entry->atributosEntrada = 0x20; //entrada do tipo arquivo
		entry->creationTime = (unsigned)time(NULL);
		entry->lastModTime = (unsigned)time(NULL);
		entry->lastAccessTime = (unsigned)time(NULL);
		entry->firstFatCluster = findFreeCluster();
		entry->fileSize1 = getLow32(cpyTam);
		entry->fileSize2 = getHigh32(cpyTam);
		
		int auxCluster = entry->firstFatCluster;
		writeFAT(auxCluster,auxCluster);
		cpyClusters.push_back(auxCluster);
		int auxCluster2;
		for(int i = 0;i<qtdClusters;i++)
		{
			auxCluster2 = findFreeCluster();
			writeFAT(auxCluster,auxCluster2);
			writeFAT(auxCluster2,0x5);
			cpyClusters.push_back(auxCluster2);
			auxCluster = auxCluster2;
			
			
		}
		writeFAT(auxCluster,0xFFFFFFFF);
		

		img = fopen(filename,"r+");
		fseek(img,entryAddr,SEEK_SET);
		fwrite(entry,sizeof(fileEntry),1,img);
		fwrite(sfn,sizeof(simpleFileName), ((tamanhoEntradaTotal/entradaArquivo)-1), img);
		fclose(img);
		
		//updateDir(parentFirstCluster,true,filename);
		
		
		
		img = fopen(filename,"r+");
		
		count = 0;
		for(auto x:cpyClusters)
		{
			fseek(img,(rootAddr+(tamCluster*x)),SEEK_SET);
			fwrite(cpyVet+(count*tamCluster), 1, tamCluster, img);
			count++;
		}
		
		fclose(img);
		
		
		
		
	}

	/*copia um arquivo do diretório atual para o disco.
	 * 
	 * parentFirstCluster: primeiro cluster do diretório atual.
	 * 
	 * cpyfilename: nome do arquivo a ser copiado.
	 * 
	 * filename: nome do arquivo de sistema.
	 */
	private: void copiarPraFora(int parentFirstCluster, char *cpyfilename)
	{
			int entryAddr = findEntry(parentFirstCluster, cpyfilename);
			
			img = fopen(filename,"r+");
			fseek(img,entryAddr+16,SEEK_SET);
			
			int firstCluster;
			long tamCpy;
			fread(&firstCluster,4,1,img);
			fread(&tamCpy,8,1,img);
			
			int aux = firstCluster;
			unsigned int aux2;
			list<int> dirClusters;
			
			do
			{
				aux2 = readFAT(aux);
				dirClusters.push_back(aux);
				aux = aux2;
				
				
				
			}while(aux2!=0xFFFFFFFF);
		
			fclose(img);
			
			int8_t *cpyVet = (int8_t*)malloc(tamCpy);
			
			int addrCluster;
			int count=0;
			img = fopen(filename, "r");
			for(auto i:dirClusters)
			{
				
				
				addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
						+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
						+ ((BS->tamanhoSetor*BS->setoresPorCluster)*i); //localização do cluster na area de dados
				
				fseek(img,addrCluster,SEEK_SET);
							
				fread(cpyVet+(count*tamCluster), 1, tamCluster, img);
				
				
				count++;
			}
			
			fclose(img);
			
			FILE *cpy = fopen(cpyfilename,"w");
			fwrite(cpyVet,1,tamCpy,cpy);
			fclose(cpy);
	}
	
	/*limpa uma entrada em um diretório.
	 *
	 * addr - endereço da entrada.
	 */
	private: void clearEntry(int addr)
	{
		int entryTam;
		
		int entryEnd;
		
		readFileName(addr+32, &entryEnd);
		
		entryTam = entryEnd-addr;
		
		img = fopen(filename,"r+");
		fseek(img,addr,SEEK_SET);
		
		int8_t *aux = (int8_t*)malloc(entryTam);
		memset(aux, 0, entryTam);
		
		fwrite(aux, entryTam, 1, img);
		
		fclose(img);
		
		
	}
	
	/*limpa um cluster inteiro (preenchendo-o com zeros).
	 *
	 * cluster - número do cluster a ser limpo.
	 */
	private: void clearCluster(int cluster)
	{
		int addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
				+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
				+ ((BS->tamanhoSetor*BS->setoresPorCluster)*cluster); //localização do cluster na area de dados
		
		img = fopen(filename,"r+");
		fseek(img,addrCluster,SEEK_SET);
		
		int8_t *aux = (int8_t*)malloc(addrCluster);
		memset(aux, 0, tamCluster);
		
		fwrite(aux, tamCluster, 1, img);
		
		fclose(img);	
				
	}
	
	/*exclui um arquivo ou diretório do sistema.
	 *
	 * curDir - o diretório onde está localizado o arquivo que será excluído.
	 * 
	 * arqName - o nome do arquivo ou diretório a ser excluído.
	 */
	private: void excluir(int curDir, char *arqName)
	{
		int entryAddr = findEntry(curDir, arqName);
		int8_t entryAttr;
		
		img = fopen(filename,"r+");
		
		fseek(img,entryAddr,SEEK_SET);
		fread(&entryAttr,1,1,img);
		fseek(img,entryAddr+16,SEEK_SET);
		int firstCluster;
		fread(&firstCluster,4,1,img);
		fclose(img);
		
		clearEntry(entryAddr);
		if(entryAttr==0x20)//excluir um arquivo.
		{
			
			int aux = firstCluster;
			unsigned int aux2;
			list<int> dirClusters;
			
			do
			{
				aux2 = readFAT(aux);
				dirClusters.push_back(aux);
				writeFAT(aux,0);
				aux = aux2;
				
				
				
			}while(aux2!=0xFFFFFFFF);
			
			for(auto i:dirClusters)
			{
				clearCluster(i);
			}
		}
		else if(entryAttr==0x10)//excluir um diretório.
		{
			
			int aux = firstCluster;
			unsigned int aux2;
			list<int> dirClusters;
			
			do
			{
				aux2 = readFAT(aux);
				dirClusters.push_back(aux);
				writeFAT(aux,0);
				aux = aux2;
				
				
				
			}while(aux2!=0xFFFFFFFF);
			
			
			int8_t entryAtt;
			bool first = true;
			for(auto i:dirClusters)
			{
				int addrCluster = (BS->tamanhoSetor*BS->setoresPorCluster) //boot record
				+ (BS->numeroTabelas*(BS->clustersPorTabela*(BS->tamanhoSetor*BS->setoresPorCluster))) //FATs
				+ ((BS->tamanhoSetor*BS->setoresPorCluster)*i); //localização do cluster na area de dados
				
				for(int j=0;j<tamCluster;j+=32)
				{
					if(first)
					{
						j+=entradaArquivo*4;
						first = false;
					}
					int aux;
					img = fopen(filename, "r+");
					fseek(img,addrCluster+j,SEEK_SET);
					fread(&entryAtt,1,1,img);

					fclose(img);
					if(entryAtt==0x20||entryAtt==0x10)
					{
						excluir(firstCluster,readFileName(addrCluster+j+32,&aux));
					}
				}
				clearCluster(i);
			}
		}
	}
};
