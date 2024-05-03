all: servidor cliente

servidor: servidor.c queue.c
	gcc servidor.c queue.c -o servidor

cliente: cliente.c
	gcc cliente.c -o cliente

clean:
	rm -f servidor cliente
	rm -rf Resultados
	rm -rf Status
