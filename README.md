# Configuração do Cluster Beowulf com OpenMPI

Este documento apresenta o processo de configuração de um cluster Beowulf em uma máquina virtual, composto por um nó mestre e quatro nós de processamento. A comunicação entre os nós é realizada por meio do OpenMPI.

## Hardware e Software Utilizados

### Hardware

- **Máquina Master**: 1 máquina virtual com 2 núcleos de CPU, 2GB de RAM e 25GB de armazenamento.
- **Nós de Processamento**: 4 máquinas virtuais, cada uma com 2 núcleos de CPU, 2GB de RAM e 25GB de armazenamento.
- **Rede**: Todas as máquinas estão conectadas via Ethernet em uma rede local.

### Software

- **Sistema Operacional**: Linux Mint instalado em todas as máquinas.
- **MPI**: OpenMPI instalado manualmente em cada máquina.
- **Configuração de Rede**: IPs estáticos configurados para cada máquina.
- **NFS**: Sistema de Arquivos de Rede configurado para compartilhar o diretório `/home/mpiuser` entre todas as máquinas.
- **SSH**: Configurado para permitir comunicação segura entre as máquinas.

## Passos para Configuração do Cluster

### 1. Instalação do Sistema Operacional

1. **Criação da Máquina Virtual**:
   - Utilizei uma imagem do Linux Mint para configurar as máquinas virtuais.
   - Instalei o Linux Mint em cada uma das máquinas virtuais, seguindo o processo padrão de instalação.

2. **Configuração dos Nomes das Máquinas**:
   - Durante a instalação, atribuí os seguintes nomes às máquinas:
     - **Master**: `master`
     - **Nós de Processamento**: `node1`, `node2`, `node3`, `node4`

### 2. Configuração de Rede

1. **Configuração de IPs Estáticos**:
   - Em cada máquina, eu crei o arquivo `/etc/netplan/99_config.yaml` para configurar IPs estáticos:
     ```yaml
     network:
       version: 2
       renderer: NetworkManager 
       ethernets:
         enp0s3: 
           dhcp4: no
           addresses:
             - 192.168.1.50/24  # IP fixo
           routes:
             - to: default
               via: 192.168.1.1  # Gateway padrão
           nameservers:
             search: [mydomain, otherdomain]
             addresses: [10.10.10.1, 1.1.1.1]
     ```
   - Para os nós, configurei os IPs como `192.168.1.51`, `192.168.1.52`, `192.168.1.53`, e `192.168.1.54`.

2. **Teste de Conexão**:
   - Utilizei o comando `ping` para verificar a conectividade entre as máquinas:
     ```bash
     ping 192.168.1.51  # Teste de conexão do master para node1
     ```

3. **Edição do Arquivo `/etc/hosts`**:
   - Em cada máquina, eu editei o arquivo `/etc/hosts`:
     ```bash
     sudo nano /etc/hosts
     ```
   - Adicionei os seguintes hosts no arquivo. Isso permite que as máquinas se reconheçam pelo nome em vez de apenas pelo endereço IP.:
     ```plaintext
     127.0.0.1       localhost
     192.168.1.50    master
     192.168.1.51    node1
     192.168.1.52    node2
     192.168.1.53    node3
     192.168.1.54    node4
     ```

### 3. Criação do Usuário MPI

1. **Criação do Usuário `mpiuser`**:
   - Em todas as máquinas, criei o usuário `mpiuser` com o mesmo UID:
     ```bash
     sudo adduser mpiuser --uid 999
     ```

### 4. Configuração do NFS (Network File System)

1. **Instalação do NFS**:
   - No master, instalei o `nfs-kernel-server`:
     ```bash
     sudo apt-get install nfs-kernel-server
     ```
   - Nos nós, instalei o `nfs-common`:
     ```bash
     sudo apt-get install nfs-common
     ```

2. **Compartilhamento do Diretório `/home/mpiuser`**:
   - No master, editei o arquivo `/etc/exports` para compartilhar o diretório:
     ```bash
     /home/mpiuser *(rw,sync,no_subtree_check)
     ```
   - Reiniciei o serviço NFS:
     ```bash
     sudo service nfs-kernel-server restart
     ```

3. **Montagem do Diretório Compartilhado nos Nós**:
   - Em cada nó, montei o diretório compartilhado:
     ```bash
     sudo mount master:/home/mpiuser /home/mpiuser
     ```
   - Adicionei a montagem automática no arquivo `/etc/fstab`:
     ```bash
     master:/home/mpiuser /home/mpiuser nfs
     ```

### 5. Configuração do SSH

1. **Instalação do SSH**:
   - Em todas as máquinas, instalei o SSH:
     ```bash
     sudo apt-get install ssh
     ```

2. **Geração de Chaves SSH**:
   - No usuário `mpiuser`, gerei chaves SSH em cada máquina:
     ```bash
     ssh-keygen -t rsa
     ```
   - Copiei as chaves públicas para o master:
     ```bash
     ssh-copy-id mpiuser@master
     ```

3. **Teste de Conexão SSH**:
   - Testei a conexão SSH do master para cada nó:
     ```bash
     ssh node1
     ```

### 6. Instalação e Configuração do OpenMPI

1. **Instalação Manual do OpenMPI**:
   - Em cada máquina, baixei e instalei o OpenMPI manualmente:
     ```bash
     wget https://download.open-mpi.org/release/open-mpi/v5.0/openmpi-5.0.6.tar.gz
     tar -xzf openmpi-5.0.6.tar.gz
     cd openmpi-5.0.6
     ./configure --prefix=/usr/local
     make
     sudo make install
     ```

2. **Configuração do Ambiente**:
   - Adicionei o caminho do OpenMPI ao `.bashrc` de cada máquina:
     ```bash
     export PATH=/usr/local/bin:$PATH
     export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
     ```

### 7. Execução de um Programa

1. **Programa MPI em C**:
   - Criei um arquivo `primos_mpi_parallel.c` no diretório `/home/mpiuser`:
     ```c
     nano primos_mpi_parallel.c
     ```
   - Compilei o programa:
     ```bash
     mpicc primos_mpi_parallel.c -o primos_mpi_parallel
     ```
   - Executei o programa no cluster:
     ```bash
     mpirun -np 8 --hostfile hosts ./primos_mpi_parallel
     ```

## Conclusão

A configuração do cluster Beowulf foi realizada com sucesso, utilizando o OpenMPI para a comunicação entre os nós. O cluster está pronto para executar tarefas paralelas, distribuindo a carga de processamento entre os nós. Esta configuração é ideal para aplicações que requerem alto desempenho e escalabilidade, como simulações científicas e processamento de grandes volumes de dados.
