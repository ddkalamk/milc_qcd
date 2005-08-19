#include <stdio.h>
#include <qdp.h>
#include <math.h>
#include "ks_imp_includes_qdp.h"
#include "RG_include.h"


void RG_value(QLA_Real *s_w, QLA_Real *l0, QLA_Int *space_only)
{
  QLA_Real d;

#ifdef CHECK
  d = 0.25; 
#else
  d = 0.13; /* value of Anna Hasenfrantz */
#endif
  *space_only = 0;
  *l0 = 1.0;
  *s_w = d/(1.0-6.0*d);


#ifdef CHECK_SMEAR_QDP_MILC
  *space_only = 1;
  *s_w = 1.0/2.5;
#endif

  return;
}

void RG_smearing(QDP_ColorMatrix *dest[RG_Nd], QDP_ColorMatrix *src[RG_Nd],QDP_Sub_Block s, int len)
{
 int i;
 QLA_Real staple_w,link0;
 QLA_Int space_only;
 QDP_ColorMatrix *temp[RG_Nd],*sm_link[RG_Nd],*pr_sm_link[RG_Nd];

  for(i=0; i< RG_Nd; ++i)
  {
    sm_link[i] = QDP_create_M();
    pr_sm_link[i] = QDP_create_M();
  }

  RG_value(&staple_w,&link0,&space_only);
  /* Two smearing steps  */

  RG_smearing_qdp(sm_link,src,&staple_w,&link0,s,len);
#ifdef CHECK_SMEAR_QDP_MILC
  project_qdp(sm_link, dest,&space_only);
#else
#ifdef CHECK_DEGRAND_W_SMEAR
  project_qdp(sm_link, dest,&space_only);
#else
#ifdef CHECK_SMEAR_GAUGE_2
  project_qdp(sm_link, dest,&space_only);
#else

  project_qdp(sm_link, pr_sm_link,&space_only);
  RG_smearing_qdp(sm_link, pr_sm_link,&staple_w,&link0,s,len);
  project_qdp(sm_link, dest,&space_only);

#endif
#endif
#endif

  for(i=0; i< RG_Nd; ++i)
  {
   QDP_destroy_M(sm_link[i]);
   QDP_destroy_M(pr_sm_link[i]);
  }

return;
}

void RG_create_gauge(QDP_ColorMatrix *res[RG_Nd], QDP_ColorMatrix *src[RG_Nd], QDP_Sub_Block s,int len)
{
  int i,j,k;
  int v[RG_Nd];
  QDP_ColorMatrix *temp,*temp1;
  QDP_Shift offset;
  

   temp = QDP_create_M();
   temp1 = QDP_create_M();

   for(j=0; j<RG_Nd; ++j)
   {
  
   for(k=0; k<RG_Nd; ++k) v[k] = 0;
   v[j] = len; 


   offset = QDP_create_shift(v);
   SQDP_M_eq_M_times_sM(res[j],src[j],src[j],offset,QDP_forward,s);
   QDP_destroy_shift(offset);
   
  // printf("Multp........node %d for %d\n",this_node,j); fflush(stdout);
   }

 // node0_printf(".......................done\n"); fflush(stdout);

  QDP_destroy_M(temp);
  QDP_destroy_M(temp1);
return;

}
  
void RG_gauge(QDP_ColorMatrix *rg_link[NRG][RG_Nd], QDP_ColorMatrix *link_qdp[RG_Nd], QDP_Sub_Block s[NRG+1])
{
  int i,j,len;
  QDP_ColorMatrix *pr_sm_link[RG_Nd];

 // node0_printf("Smearing links with Degrand trick........\n"); fflush(stdout);
 
  for(i=0; i< RG_Nd; ++i)
    pr_sm_link[i] = QDP_create_M();


#ifdef CHECK_DEGRAND_WO_SMEAR
  for(i=0; i< RG_Nd; ++i)
  SQDP_M_eq_M(rg_link[NRG-1][i],link_qdp[i],s[NRG]);
#else
  RG_smearing(rg_link[NRG-1],link_qdp,s[NRG],1);
#ifdef CHECK_DEGRAND_W_SMEAR
  SQDP_M_eq_M(rg_link[NRG-1][3],link_qdp[3],s[NRG]);
#endif
#endif


  for (i=1;i<NRG;i++)
  {
  len = intpow(2,i-1);

 //  printf("node %d: Smear links of length %d x a'\n",this_node,len); fflush(stdout);
 //  printf("node %d: rg_links of length %d x a'\n",this_node,2*len); fflush(stdout);

#ifdef CHECK_DEGRAND_WO_SMEAR
  for(j=0; j< RG_Nd; ++j)
   SQDP_M_eq_M(pr_sm_link[j],rg_link[NRG-i][j],s[NRG-i+1]);
#else
  RG_smearing(pr_sm_link,rg_link[NRG-i],s[NRG-i+1],len);
#ifdef CHECK_DEGRAND_W_SMEAR
  SQDP_M_eq_M(pr_sm_link[3],rg_link[NRG-i][3],s[NRG-i+1]);
#endif
#endif

  RG_create_gauge(rg_link[NRG-i-1],pr_sm_link,s[NRG-i],len);

  }
 

//  node0_printf(".......................done\n"); fflush(stdout);
  for(i=0; i< RG_Nd; ++i)
   QDP_destroy_M(pr_sm_link[i]);

return;

}


