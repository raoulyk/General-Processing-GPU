__kernel void MetalPlate(__global double* vector, __global double* copy_vector, int M, int N, int x0, int y0) {

	int y = get_global_id(0);
	int x = get_global_id(1);

	if (x == 0 || x == M - 1 || y == 0 || y == N - 1 || (x == (x0 + 1) && y == (y0 + 1))) {
		return;
	} 

	double mean = 0;
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				 mean += copy_vector[(x + i) * N + y + j];
			}
		}
	vector[x * N + y] = mean / 9;
}

