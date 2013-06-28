all: change-interp

%: %.c
	gcc -o $@ $^ -lelf

clean:
	rm -rf change-interp
