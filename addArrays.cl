__kernel void addArrays_kernel(__global const int *a,
						__global const int *b,
						__global int *res)
{
    int gid = get_global_id(0);

    res[gid] = a[gid] + b[gid];
}