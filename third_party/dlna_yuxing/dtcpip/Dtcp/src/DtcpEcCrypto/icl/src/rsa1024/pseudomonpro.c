#if 0
void __stdcall MontProduct (ICLWord n0prime, RSAData m,
	RSAData *a, RSAInt *b, RSAData *t)
{
	int s = m.length;
	for (int i = 0; i < s; ++i) {
		t = b * a[i];
		f = lowhalf(m[0] * n0prime);
		t = (t + (m * f)) >> (8*sizeof(ICLWord));
	}
	return (t < m) ? t : t - m;
}
#endif
