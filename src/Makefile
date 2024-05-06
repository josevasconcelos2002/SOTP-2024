all: orchestrator client

orchestrator: orchestrator.c queue.c
	gcc orchestrator.c queue.c queue2.c -o orchestrator

client: client.c
	gcc client.c -o client

clean:
	rm -f orchestrator client
	rm -rf Resultados
	rm -rf Status
