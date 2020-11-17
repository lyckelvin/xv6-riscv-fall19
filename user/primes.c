#include "kernel/types.h"
#include "user/user.h"

void source() {
	int i;
	for (i = 2; i < 36; i++) {
		write(1, &i, sizeof(i));
	}
}

void cull(int i) {
	int n;
	while (read(0, &n, sizeof(n))) {
		if (n % i != 0) {
			write(1, &n, sizeof(n));
		}
	}
}

void redirect(int k, int p[]) {
	close(k);
	dup(p[k]);
	close(p[0]);
	close(p[1]);
}

void sink() {
	int p[2];
	int i;

	if (read(0, &i, sizeof(i))) {
		printf("prime %d\n", i);
		pipe(p);
		if (fork()) {  
			redirect(0, p);
			sink();
		} else {
			redirect(1, p);
			cull(i);
		}
	}
}

int main(int argc, char *argv[]) {

	int p[2];
	pipe(p);

	if (fork()) {
		redirect(0, p);
		sink();
	} else {
		redirect(1, p);
		source();
	}

	exit();
}	