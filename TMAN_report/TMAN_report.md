André Alves 88811\
Eduardo Coelho 88867 

# Task Management framework for FreeRTOS 

## Descrição das estruturas de dados usadas 

Para as tarefas foi criado um *struct* denominado *task\_tman* com os seguintes atributos:

- char name[16]: nome da tarefa
- int period: período da tarefa definido com TaskRegisterAttributes()
- int phase: fase da tarefa definido com TaskRegisterAttributes()
- int deadline: deadline da tarefa definido com TaskRegisterAttributes()
- int deadline\_misses: contador de deadline misses para estatística
- char precedence[16]: nome da tarefa da qual depende, definido com TaskRegisterAttributes()
- int num\_activations: contador de número de ativações para estatística
- int last\_activation: *tick* da última ativação para calcular se houve deadline miss
- Semaphorehandle\_t semaphore: semáforo utilizado pelas tarefas que dependem desta
- int is\_precedent: *flag* para caso esta tarefa tenha dependentes, para saber que tem de dar give

De modo a armazenar estas tarefas foi criado um array estático com tamanho definido na variável ‘ARRAY\_SIZE’, que no caso do problema está inicializada com valor 6.

Foram também utilizados semáforos binários, presentes na *struct* referida anteriormente, para implementar a precedência simples. 

## Funcionalidades implementadas (obrigatórias) 

Todas as funcionalidades obrigatórias foram implementadas: suporte básico para ativação de tarefas periódicas, possibilidade de atribuir fase relativa e deadline relativa a uma tarefa, e possibilidade de definir precedência simples entre duas tarefas. Explicando a implementação: 

### Suporte básico para ativação de tarefas periódicas:

De modo a garantir a ativação periódica das tarefas foi primeiro necessário escolher uma forma de suspender e acordar as tarefas. Para este propósito foram utilizados os métodos da framework FreeRTOS: *vTaskSuspend()* e *vTaskResume().*  

Após ser inicializada a tarefa cuja função é ser o *tick handler*, ela percorre todas as tarefas guardadas no array e verifica a seguinte condição: “(*xTaskGetTickCount()* % (*task.Period* \* *tman\_period*)) - (*task.Phase* \* *tman\_period*) == 0”, sendo *tman\_period* o período após o qual é incrementado os ticks internos do tman, definido com o valor do argumento da função *TMAN\_Init()*. Caso esta condição se verifique, a tarefa, que está suspensa dentro do método *TaskWaitPeriod()*, irá ser resumida. 

### Possibilidade de atribuir fase relativa e deadline relativa: 

Foram adicionados os atributos *phase* e *deadline* ao *struct* da tarefa e foi implementada a possibilidade de definir estes mesmos atributos através da função TaskRegisterAttributes (e.g. *TMAN\_TaskRegisterAttributes(“D”, “PHASE”, 1)*). 

### Precedência simples: 

Uma tarefa ao registar o seu atributo *precedence* com o nome de outra tarefa irá inicializar o atributo *semaphore* dessa outra tarefa e consequentemente a variável *is\_precedent*. 

Durante o *TaskWaitPeriod()*, a tarefa após ser acordada do *vTaskSuspend()* vai verificar se depende de alguma outra tarefa verificando o seu atributo *precedence* e, se depender, irá então fazer *xSemaphoreTake()* do *semaphore* dessa outra tarefa.  

Por outro lado, uma tarefa que tenha o seu atributo *is\_precedent* a 1 irá dar *xSemaphoreGive()* ao seu *semaphore* após executar.

## Funcionalidades implementadas (opcionais) 

Das funcionalidades opcionais propostas apenas foi implementada a possibilidade de detetar e contar deadline misses, com posterior apresentação do valor na função *TaskStats*. Explicando a implementação:

### Detetar deadline misses: 

Logo após a tarefa deixar de estar suspensa, é atualizado o seu atributo *last\_activation* para o valor de *tman\_ticks* atual. 

No início do *TaskWaitPeriod()*, antes de suspender a tarefa, ou seja, logo após a execução da mesma, é verificada a seguinte condição: `“tman\_ticks* – *task.last\_activation* > *task.deadline*`. Caso esta condição se verifique significa que a deadline relativa da tarefa foi violada e, portanto, o seu atributo *deadline\_misses* é incrementado. 

## Descrição da API implementada 

### Function: TMAN\_Init() 

- Input: tick\_ms (int) 
- Returns: TMAN\_SUCCESS (0) após inicializar a framework. 
- Overview: Inicializa a framework, criando uma tarefa com o período (tick\_ms) em milisegundos para lidar com os ticks da framework e ativar as tarefas adicionadas à framework.  

### Function: TMAN\_Close() 

- Input:  
- Returns: TMAN\_SUCCESS (0) após terminar a framework. 
- Overview: Termina a framework, terminando com o escalonador de tarefas do FreeRTOS.  

### Function:   TMAN\_TaskAdd() 

- Input: taskName (char[]) 
- Returns: TMAN\_SUCCESS (0) após adicionar uma tarefa. 
- Overview:  Adiciona uma tarefa ao escalonador  

### Function: TMAN\_TaskRegisterAttributes() 

- Input:  
- taskName (char[]); 
- attribute (char[]); 
- value (char[]). 
- Attributes: PERIOD, PHASE, DEADLINE, PRECEDENCE 
- Returns:  
- TMAN\_SUCCESS (0) após registar um atributo numa tarefa. 
- TMAN\_FAIL\_INVALID\_ATTRIBUTE (-2) caso o atributo (attribute) selecionado não existir. 
- TMAN\_FAIL\_TASK\_NOT\_ADDED (-3) caso a tarefa não tenha sido adicionada à framework 
- TMAN\_FAIL (-1) para outros erros. 
- Overview: Regista o valor (value) dos atributos (PERIOD, PHASE, DEADLINE, PRECEDENCE) para a tarefa (taskName) indicada se esta já tiver sido adicionada à framework. 

### Function: TMAN\_TaskWaitPeriod() 

- Input: pvParameters (char \*) 
- Returns:  
- Overview: É chamada por uma tarefa (pvParameters) para sinalizar o fim de uma instância e esperar pela próxima ativação. Regista o número de ativações, o tick da última ativação e deteta se falhou a sua deadline, registando se for o caso, para a tarefa que a chama. Se tiver precedência ou for precedente são realizadas as ações acima descritas em casos de precedência. 

### Function: TMAN\_TaskStats() 

- Input: taskName (char[]) 
- Returns: int[2] 
- Overview: Retorna o número de ativações e o número de falhas de deadline da tarefa que chama num array com 2 posições. 

## Descrição dos testes executados e dos resultados obtidos 

Realizando um teste com a seguinte configuração e com o tick da framework com período de 200ms: 

| Task | Priority | Period | Phase | Dependencies |
|:-:|:-:|:-:|:-:|:-:|
| A | 4 | 1 | 0 | |
| B | 4 | 1 | 0 | F | 
| C | 3 | 3 | 0 | |
| D | 3 | 3 | 1 | |
| E | 2 | 4 | 0 | |
| F | 2 | 4 | 2 | |
#### Os Resultados foram os seguintes:
![](test_a.png)
![](test_b.png)
![](test_c.png)
![](test_d.png) 

Podemos verificar que todas as tarefas são  executadas com o período e fase corretos, que respeitam a prioridade (executando as de maior prioridade primeiro), e que respeitam as precedências, que é o caso da tarefa B que apenas é executada após a tarefa F. Como a tarefa B tem um período mais baixo que a tarefa F e é ativada primeiro falha sempre a deadline como é mostrado na função TMAN\_TaskStatus. 
