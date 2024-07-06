all: client ss nm

client:
	gcc -w -o client client.c
ss:
	gcc -w -o ss ss.c fileop.c
nm:
	gcc -w -o nm nm.c lru.c tries.c
