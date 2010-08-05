#include <stdio.h>

// This is a VERY basic standard library for Nava

extern "C" {
	double print(double data) {
		printf("%f\n", data);
		return data;
	}

	double read() {
		float a = 0; // It's 2am, i'll deal with this tomorrow :D
		printf("Your input : ");
		scanf("%f", &a);
		return a;
	}
}
