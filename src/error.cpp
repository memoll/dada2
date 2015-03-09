#include <Rcpp.h>
#include "dada.h"
using namespace Rcpp;

// [[Rcpp::interfaces(r, cpp)]]

Rcpp::DataFrame b_get_quality_subs(B *b) {
  int i, f, r, s, pos, qual, nti0, nti1, sub_ind;
  Sub *sub;
  Raw *raw;
  
  // Storage for counts for each qual and each nti->ntj
  int32_t nts_by_qual[1 + 4][QMAX-QMIN+1] = {0};
  int32_t subs_by_qual[1 + 16][QMAX-QMIN+1] = {0};
  
  if(b->use_quals) {
    for(i=0;i<b->nclust;i++) {
      for(f=0;f<b->bi[i]->nfam;f++) {
        for(r=0;r<b->bi[i]->fam[f]->nraw;r++) {
          raw = b->bi[i]->fam[f]->raw[r];
          sub = b->bi[i]->sub[raw->index]; // Using sub-specific raw for eventual pos1
          for(pos=0;pos<strlen(raw->seq);pos++) {
            nti1 = (int) (raw->seq[pos] - 1);
            qual = round(raw->qual[pos]);
            nts_by_qual[0][qual-QMIN] += raw->reads;
            nts_by_qual[1+nti1][qual-QMIN] += raw->reads;
            // NOTE: This _should_ be incrementing nti0, the source nt, but we dont know what
            // that is yet. This is corrected for subs below for subs (where they differ).
          }
          if(sub) {
            for(s=0;s<sub->nsubs;s++) {
              nti0 = (int) (sub->nt0[s]-1);
              nti1 = (int) (sub->nt1[s]-1);
              qual = round(raw->qual[sub->pos1[s]]);
              if(qual != round(sub->q1[s])) {
                printf("Warning: Different quals: %i, %i\n", qual, round(sub->q1[s]));
              }
              // Record the sub
              subs_by_qual[0][qual-QMIN] += raw->reads;
              sub_ind = 1 + 4*nti0 + nti1;
              subs_by_qual[sub_ind][qual-QMIN] += raw->reads;
              // Correct the source nt
              nts_by_qual[1+nti1][qual-QMIN] -= raw->reads;
              nts_by_qual[1+nti0][qual-QMIN] += raw->reads;
            }
          }
        } // for(r=0;b->bi[i]->fam[f]->nraw)
      } // for(f=0;f<bb->bi[i]->nfam;f++)
    }
  } // if(b->use_quals)

  Rcpp::NumericVector Rnts_by_qual;
  Rcpp::NumericVector RAs_by_qual;
  Rcpp::NumericVector RCs_by_qual;
  Rcpp::NumericVector RGs_by_qual;
  Rcpp::NumericVector RTs_by_qual;
  
  Rcpp::NumericVector Rsubs_by_qual;
  Rcpp::NumericVector RAAs_by_qual;
  Rcpp::NumericVector RACs_by_qual;
  Rcpp::NumericVector RAGs_by_qual;
  Rcpp::NumericVector RATs_by_qual;
  Rcpp::NumericVector RCAs_by_qual;
  Rcpp::NumericVector RCCs_by_qual;
  Rcpp::NumericVector RCGs_by_qual;
  Rcpp::NumericVector RCTs_by_qual;
  Rcpp::NumericVector RGAs_by_qual;
  Rcpp::NumericVector RGCs_by_qual;
  Rcpp::NumericVector RGGs_by_qual;
  Rcpp::NumericVector RGTs_by_qual;
  Rcpp::NumericVector RTAs_by_qual;
  Rcpp::NumericVector RTCs_by_qual;
  Rcpp::NumericVector RTGs_by_qual;
  Rcpp::NumericVector RTTs_by_qual;
  for(i=0;i<QMAX-QMIN+1;i++) {
    Rnts_by_qual.push_back(nts_by_qual[0][i]);
    RAs_by_qual.push_back(nts_by_qual[1][i]);
    RCs_by_qual.push_back(nts_by_qual[2][i]);
    RGs_by_qual.push_back(nts_by_qual[3][i]);
    RTs_by_qual.push_back(nts_by_qual[4][i]);
    
    Rsubs_by_qual.push_back(subs_by_qual[0][i]);
    RAAs_by_qual.push_back(nts_by_qual[1][i] - subs_by_qual[2][i] - subs_by_qual[3][i] - subs_by_qual[4][i]);
    RACs_by_qual.push_back(subs_by_qual[2][i]);
    RAGs_by_qual.push_back(subs_by_qual[3][i]);
    RATs_by_qual.push_back(subs_by_qual[4][i]);

    RCAs_by_qual.push_back(subs_by_qual[5][i]);
    RCCs_by_qual.push_back(nts_by_qual[2][i] - subs_by_qual[5][i] - subs_by_qual[7][i] - subs_by_qual[8][i]);
    RCGs_by_qual.push_back(subs_by_qual[7][i]);
    RCTs_by_qual.push_back(subs_by_qual[8][i]);

    RGAs_by_qual.push_back(subs_by_qual[9][i]);
    RGCs_by_qual.push_back(subs_by_qual[10][i]);
    RGGs_by_qual.push_back(nts_by_qual[3][i] - subs_by_qual[9][i] - subs_by_qual[10][i] - subs_by_qual[12][i]);
    RGTs_by_qual.push_back(subs_by_qual[12][i]);
    
    RTAs_by_qual.push_back(subs_by_qual[13][i]);
    RTCs_by_qual.push_back(subs_by_qual[14][i]);
    RTGs_by_qual.push_back(subs_by_qual[15][i]);
    RTTs_by_qual.push_back(nts_by_qual[4][i] - subs_by_qual[13][i] - subs_by_qual[14][i] - subs_by_qual[15][i]);
  }  

  Rcpp::NumericVector qq(QMAX-QMIN+1);
  for(i=QMIN;i<=QMAX;i++) { qq[i-QMIN] = i; }

  
/*  Rcpp::NumericVector qq(41);
  for(i=0;i<=40;i++) { qq[i] = i; }
  Rcpp::NumericVector n_unq(41);
  Rcpp::NumericVector n_read(41);
  Rcpp::NumericVector n_unq_sub(41);
  Rcpp::NumericVector n_read_sub(41); // initializes to zero

  if(b->use_quals) {
    for(i=0;i<b->nclust;i++) {
      for(f=0;f<b->bi[i]->nfam;f++) {
        sub = b->bi[i]->fam[f]->sub;
        for(r=0;r<b->bi[i]->fam[f]->nraw;r++) {
          raw = b->bi[i]->fam[f]->raw[r];
          for(pos=0;pos<strlen(raw->seq);pos++) {
            qual = round(raw->qual[pos]);
            n_unq[qual]++;
            n_read[qual] += raw->reads;
          }
          if(sub) {
            for(s=0;s<sub->nsubs;s++) {
              qual = round(raw->qual[sub->pos[s]]);
              n_unq_sub[qual]++;
              n_read_sub[qual] += raw->reads;
            }
          }
        } // for(r=0;b->bi[i]->fam[f]->nraw)
      }
    } // for(i=0;i<b->nclust;i++)
  }*/
  Rcpp::DataFrame df_subs = Rcpp::DataFrame::create(_["AveQ"]= qq, _["nts"] = Rnts_by_qual, _["subs"] = Rsubs_by_qual, 
      _["A"] = RAs_by_qual, _["C"] = RCs_by_qual, _["G"] = RGs_by_qual, _["T"] = RTs_by_qual,
      _["A2C"] = RACs_by_qual, _["A2G"] = RAGs_by_qual, _["A2T"] = RATs_by_qual, 
      _["C2A"] = RCAs_by_qual, _["C2G"] = RCGs_by_qual, _["C2T"] = RCTs_by_qual,
      _["G2A"] = RGAs_by_qual, _["G2C"] = RGCs_by_qual, _["G2T"] = RGTs_by_qual,
      _["T2A"] = RTAs_by_qual, _["T2C"] = RTCs_by_qual, _["T2G"] = RTGs_by_qual);  // Max 20 cols in Rcpp::DataFrame::create
  
  return(df_subs);
}

// Function to get the number of transitions observed for each possible nt combo
void b_get_trans_matrix(B *b, int32_t obs[4][4]) {
  int nti0, nti1, i, j, f, s;
  int32_t total = 0; // Will be used to check for overflows
  int32_t prev;
  Fam *fam;
  
  // Initialize obs - no pseudocounts
  for(i=0;i<4;i++) {
    for(j=0;j<4;j++) {
      obs[i][j] = 0;
    }
  }
  
  // Count up all observed transitions
  for(i=0;i<b->nclust;i++) {
    // Initially add all counts to the no-error slots
    for(j=0;j<strlen(b->bi[i]->seq);j++) {
      nti0 = (int) (b->bi[i]->seq[j] - 1);
      obs[nti0][nti0] += b->bi[i]->reads;
      
      prev = total;
      total += b->bi[i]->reads;
      if(total < prev) { // OVERFLOW
        printf("OVERFLOW IN b_get_trans_matrix!!!!!!\n");
      }
    }
    
    // Move counts corresponding to each substitution with the fams
    for(f=0;f<b->bi[i]->nfam;f++) {
      fam = b->bi[i]->fam[f];
      if(fam->sub) { // Not NULL sub
        for(s=0;s<fam->sub->nsubs;s++) {
          nti0 = (int) (fam->sub->nt0[s]-1);
          nti1 = (int) (fam->sub->nt1[s]-1);
          obs[nti0][nti0] -= fam->reads;
          obs[nti0][nti1] += fam->reads;
        }
      }
    }
  } // for(i=0;i<b->nclust;i++)
}

Rcpp::DataFrame b_get_positional_subs(B *b) {
  //! CONSTRAIN THIS TO THE MAXIMUM LENGTH OF THE SEQUENCES
  int i, j, f, s, sub_ind;
  Fam *fam;
  Sub *sub;
  int32_t nts_by_pos[1+4][SEQLEN] = {0};
  int32_t subs_by_pos[1+16][SEQLEN] = {0};
  
  for(i=0;i<b->nclust;i++) {
    for(j=0;j<strlen(b->bi[i]->seq);j++) {
      nts_by_pos[0][j] += b->bi[i]->reads;
      nts_by_pos[(int) b->bi[i]->seq[j]][j] += b->bi[i]->reads;
    }
    
    for(f=0;f<b->bi[i]->nfam;f++) {
      fam = b->bi[i]->fam[f];
      sub = fam->sub;
      if(sub) { // not a NULL sub
        for(s=0;s<sub->nsubs;s++) {
          subs_by_pos[0][sub->pos[s]] += fam->reads;
          sub_ind = 1 + 4* ((int) sub->nt0[s] - 1) + ((int) sub->nt1[s] - 1);
          subs_by_pos[sub_ind][sub->pos[s]] += fam->reads;
        }
      } else {  // Fams should never have NULL subs
        printf("Warning: Output fam C%iF%i had a NULL sub.\n", i, f);
      }
    } // for(f=0;f<bb->bi[i]->nfam;f++)
  }
  
  Rcpp::NumericVector Rnts_by_pos;
  Rcpp::NumericVector RAs_by_pos;
  Rcpp::NumericVector RCs_by_pos;
  Rcpp::NumericVector RGs_by_pos;
  Rcpp::NumericVector RTs_by_pos;
  
  Rcpp::NumericVector Rsubs_by_pos;
  Rcpp::NumericVector RAAs_by_pos;
  Rcpp::NumericVector RACs_by_pos;
  Rcpp::NumericVector RAGs_by_pos;
  Rcpp::NumericVector RATs_by_pos;
  Rcpp::NumericVector RCAs_by_pos;
  Rcpp::NumericVector RCCs_by_pos;
  Rcpp::NumericVector RCGs_by_pos;
  Rcpp::NumericVector RCTs_by_pos;
  Rcpp::NumericVector RGAs_by_pos;
  Rcpp::NumericVector RGCs_by_pos;
  Rcpp::NumericVector RGGs_by_pos;
  Rcpp::NumericVector RGTs_by_pos;
  Rcpp::NumericVector RTAs_by_pos;
  Rcpp::NumericVector RTCs_by_pos;
  Rcpp::NumericVector RTGs_by_pos;
  Rcpp::NumericVector RTTs_by_pos;
  for(i=0;i<SEQLEN;i++) {
    if(nts_by_pos[i]==0) { break; }
    else {
      Rnts_by_pos.push_back(nts_by_pos[0][i]);
      RAs_by_pos.push_back(nts_by_pos[1][i]);
      RCs_by_pos.push_back(nts_by_pos[2][i]);
      RGs_by_pos.push_back(nts_by_pos[3][i]);
      RTs_by_pos.push_back(nts_by_pos[4][i]);
      
      Rsubs_by_pos.push_back(subs_by_pos[0][i]);
      RAAs_by_pos.push_back(nts_by_pos[1][i] - subs_by_pos[2][i] - subs_by_pos[3][i] - subs_by_pos[4][i]);
      RACs_by_pos.push_back(subs_by_pos[2][i]);
      RAGs_by_pos.push_back(subs_by_pos[3][i]);
      RATs_by_pos.push_back(subs_by_pos[4][i]);

      RCAs_by_pos.push_back(subs_by_pos[5][i]);
      RCCs_by_pos.push_back(nts_by_pos[2][i] - subs_by_pos[5][i] - subs_by_pos[7][i] - subs_by_pos[8][i]);
      RCGs_by_pos.push_back(subs_by_pos[7][i]);
      RCTs_by_pos.push_back(subs_by_pos[8][i]);

      RGAs_by_pos.push_back(subs_by_pos[9][i]);
      RGCs_by_pos.push_back(subs_by_pos[10][i]);
      RGGs_by_pos.push_back(nts_by_pos[3][i] - subs_by_pos[9][i] - subs_by_pos[10][i] - subs_by_pos[12][i]);
      RGTs_by_pos.push_back(subs_by_pos[12][i]);
      
      RTAs_by_pos.push_back(subs_by_pos[13][i]);
      RTCs_by_pos.push_back(subs_by_pos[14][i]);
      RTGs_by_pos.push_back(subs_by_pos[15][i]);
      RTTs_by_pos.push_back(nts_by_pos[4][i] - subs_by_pos[13][i] - subs_by_pos[14][i] - subs_by_pos[15][i]);
    }
  }
  Rcpp::DataFrame df_subs = Rcpp::DataFrame::create(_["nts"] = Rnts_by_pos, _["subs"] = Rsubs_by_pos, 
      _["A"] = RAs_by_pos, _["C"] = RCs_by_pos, _["G"] = RGs_by_pos, _["T"] = RTs_by_pos,
      _["A2C"] = RACs_by_pos, _["A2G"] = RAGs_by_pos, _["A2T"] = RATs_by_pos, 
      _["C2A"] = RCAs_by_pos, _["C2G"] = RCGs_by_pos, _["C2T"] = RCTs_by_pos,
      _["G2A"] = RGAs_by_pos, _["G2C"] = RGCs_by_pos, _["G2T"] = RGTs_by_pos,
      _["T2A"] = RTAs_by_pos, _["T2C"] = RTCs_by_pos, _["T2G"] = RTGs_by_pos);  // Max 20 cols in Rcpp::DataFrame::create
  return(df_subs);
}
