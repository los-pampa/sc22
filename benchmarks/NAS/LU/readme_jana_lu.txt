-------------------------------
Perfil de #pramga do LU (NAS):
-------------------------------

dentro de sssor()
----------------------------------------------------------------------
blts.c // chamado no sssor()
	blts()
		#pragma omp for schedule(static) nowait
		for (j = jst; j < jend; j++) {
		
		#pragma omp for schedule(static) nowait
		for (j = jst; j < jend; j++) {
		
buts.c // chamado no sssor()
	buts()
		  #pragma omp for schedule(static) nowait
		  for (j = jend - 1; j >= jst; j--) {

		#pragma omp for schedule(static) nowait
  		for (j = jend - 1; j >= jst; j--) {
		
jacld.c  // chamado no sssor()
	jacld()
		#pragma omp for schedule(static) nowait
		for (j = jst; j < jend; j++) {
		
jacu.c  // chamado no sssor()
	jacu()
		#pragma omp for schedule(static) nowait
		for (j = jend - 1; j >= jst; j--) {
--------------------------------------------------------------------		
		
l2norm.c // chamado 3x no sssor()
	l2norm()
		#pragma omp parallel default(shared) private(i,j,k,m,sum_local)
			#pragma omp for nowait
			#pragma omp atomic
			
rhs.c // chamado 2x no sssor()
	rhs()
		#pragma omp parallel default(shared) private(i,j,k,m,q,flux,tmp,utmp,rtmp,\
              u51im1,u41im1,u31im1,u21im1,u51i,u41i,u31i,u21i,u21, \
              u51jm1,u41jm1,u31jm1,u21jm1,u51j,u41j,u31j,u21j,u31, \
              u51km1,u41km1,u31km1,u21km1,u51k,u41k,u31k,u21k,u41)
	  	{
  			#pragma omp for schedule(static)
			#pragma omp master
			#pragma omp for schedule(static) nowait
			...
			

================		
SO MEDIR, nao adaptar
erhs.c // chamado UMA unica vez no lu
	erhs()
		  #pragma omp parallel default(shared) private(i,j,k,m,xi,eta,zeta,tmp,q,\
              flux,u51im1,u41im1,u31im1,u21im1,u51i,u41i,u31i,u21i,u21,\
                   u51jm1,u41jm1,u31jm1,u21jm1,u51j,u41j,u31j,u21j,u31,\
                   u51km1,u41km1,u31km1,u21km1,u51k,u41k,u31k,u21k,u41)
		  {
  				#pragma omp for schedule(static) nowait
   			    #pragma omp for schedule(static)
				#pragma omp for schedule(static) nowait
				#pragma omp for schedule(static)
				#pragma omp for schedule(static) nowait
		  }

error.c // chamado UMA unica vez no lu
	error()
		#pragma omp parallel default(shared) private(i,j,k,m,tmp,u000ijk,errnm_local)
			#pragma omp for nowait
			#pragma omp atomic
				
pintgr.c // chamado UMA unica vez no lu
	pintgr()
		#pragma omp parallel default(shared) private(i,j,k) \
                       shared(ki1,ki2,ifin,ibeg,jfin,jbeg,ifin1,jfin1)
  		{
  			#pragma omp for nowait
			#pragma omp single
			#pragma omp for reduction(+:frc1)
			#pragma omp single nowait
			#pragma omp for nowait
			#pragma omp for nowait
			#pragma omp single
			#pragma omp for reduction(+:frc2)
			#pragma omp single nowait
			#pragma omp for nowait
			#pragma omp for nowait
			#pragma omp single
			#pragma omp for reduction(+:frc3)
			#pragma omp single nowait
		
setbv.c // chamado 2x no lu
	setbv()
		#pragma omp parallel default(shared) private(i,j,k,m,temp1,temp2) \
                                       shared(nx,ny,nz)
  		{
  			#pragma omp for schedule(static)
  			for (j = 0; j < ny; j++) {
			#pragma omp for schedule(static) nowait
			for (k = 0; k < nz; k++) {
			#pragma omp for schedule(static) nowait
			for (k = 0; k < nz; k++) {

setiv.c // chamado 2x no lu
	setiv()
		  #pragma omp parallel for default(shared) private(i,j,k,m,pxi,peta,pzeta, \
              xi,eta,zeta,ue_ijnz,ue_ij1,ue_iny0k,ue_i1k,ue_nx0jk,ue_1jk)  \
              shared(nx0,ny0,nz)
			  for (k = 1; k < nz - 1; k++) {

ssor.c // chamado no lu
	ssor()
	// isso acontece so duas x
		#pragma omp parallel default(shared) private(m,n,i,j)
			#pragma omp for nowait
			#pragma omp for nowait
			
	// so medir		
		rhs();
		l2norm();

	for (niter) {
// essa regiao eh que acontece mais vezes (e que se repete, valendo a penas adaptar...
		#pragma omp parallel default(shared) private(i,j,k,m,tmp2) \
                shared(ist,iend,jst,jend,nx,ny,nz,nx0,ny0,omega)
				for
					jacld(k);
					blts
				for
					jacu
					buts
		}
		
	// so medir		
		l2norm
		rhs
		l2norm
		
		
syncs.c
	sync_left()
		#pragma omp flush(isync)
		#pragma omp flush(isync,v)
	sync_right()
		#pragma omp flush(isync,v)
		#pragma omp flush(isync)
		#pragma omp flush(isync)
	

===============================
lu ()

	// ini so medir
  setbv();
  setiv();
  erhs();
	// fim so medir

  ssor(1);
  	

	// ini so medir
  setbv();
  setiv();
	// fim so medir

  ssor(itmax);

	// ini so medir
  error();
  pintgr();
	// fim so medir

			
		
		