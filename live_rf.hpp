#pragma once
#include <math.h>

// CapiGrad Live Advisor (Random Forest Auto-Generated)
#include <string.h>
void add_vectors(double *v1, double *v2, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] + v2[i];
}
void mul_vector_number(double *v1, double num, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] * num;
}
inline void live_rf_score(double * input, double * output) {
    double var0[3];
    double var1[3];
    double var2[3];
    double var3[3];
    double var4[3];
    double var5[3];
    double var6[3];
    double var7[3];
    double var8[3];
    double var9[3];
    double var10[3];
    double var11[3];
    double var12[3];
    double var13[3];
    double var14[3];
    double var15[3];
    double var16[3];
    double var17[3];
    double var18[3];
    double var19[3];
    double var20[3];
    double var21[3];
    double var22[3];
    double var23[3];
    double var24[3];
    double var25[3];
    double var26[3];
    double var27[3];
    double var28[3];
    double var29[3];
    double var30[3];
    double var31[3];
    double var32[3];
    double var33[3];
    double var34[3];
    double var35[3];
    double var36[3];
    double var37[3];
    double var38[3];
    double var39[3];
    double var40[3];
    double var41[3];
    double var42[3];
    double var43[3];
    double var44[3];
    double var45[3];
    double var46[3];
    double var47[3];
    double var48[3];
    double var49[3];
    double var50[3];
    if (input[2] <= 0.06509999930858612) {
        if (input[1] <= -0.4826499968767166) {
            memcpy(var50, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= 0.03645000047981739) {
                if (input[0] <= 1.25) {
                    memcpy(var50, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.3186499923467636) {
                        if (input[1] <= -0.06369999796152115) {
                            memcpy(var50, __extension__ (const double[]){0.42500000000000004, 0.35000000000000003, 0.22500000000000003}, 3 * sizeof(double));
                        } else {
                            memcpy(var50, __extension__ (const double[]){0.6891891891891891, 0.02702702702702703, 0.28378378378378377}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.1847500056028366) {
                            memcpy(var50, __extension__ (const double[]){0.42595204513399154, 0.15162200282087446, 0.422425952045134}, 3 * sizeof(double));
                        } else {
                            memcpy(var50, __extension__ (const double[]){0.46986541837331774, 0.12346401404330018, 0.4066705675833821}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 0.06039999984204769) {
                    if (input[1] <= -0.368149995803833) {
                        memcpy(var50, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= 0.05454999953508377) {
                            memcpy(var50, __extension__ (const double[]){0.2, 0.05, 0.75}, 3 * sizeof(double));
                        } else {
                            memcpy(var50, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var50, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        memcpy(var50, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    double var51[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var51, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.37880000472068787) {
            if (input[0] <= 1.25) {
                memcpy(var51, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[2] <= -0.35309998691082) {
                    if (input[2] <= -0.35439999401569366) {
                        if (input[3] <= -0.45419999957084656) {
                            memcpy(var51, __extension__ (const double[]){0.16666666666666666, 0.5, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var51, __extension__ (const double[]){0.75, 0.08333333333333333, 0.16666666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var51, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= -0.34814999997615814) {
                        memcpy(var51, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= -0.31575000286102295) {
                            memcpy(var51, __extension__ (const double[]){0.5784313725490197, 0.06862745098039216, 0.35294117647058826}, 3 * sizeof(double));
                        } else {
                            memcpy(var51, __extension__ (const double[]){0.4549586776859504, 0.1305785123966942, 0.41446280991735535}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var51, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var50, var51, 3, var49);
    double var52[3];
    if (input[3] <= 0.06509999930858612) {
        if (input[3] <= -0.3948500007390976) {
            if (input[1] <= -0.14525000005960464) {
                if (input[0] <= 1.0450000017881393) {
                    memcpy(var52, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= -0.27709999680519104) {
                        if (input[3] <= -0.5361999869346619) {
                            memcpy(var52, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var52, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var52, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[1] <= 0.03394999913871288) {
                    memcpy(var52, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var52, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[0] <= 1.25) {
                memcpy(var52, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[3] <= -0.08559999987483025) {
                    if (input[2] <= -0.13475000113248825) {
                        if (input[1] <= -0.06234999932348728) {
                            memcpy(var52, __extension__ (const double[]){0.4334795321637427, 0.18201754385964913, 0.3845029239766082}, 3 * sizeof(double));
                        } else {
                            memcpy(var52, __extension__ (const double[]){0.4410911201392919, 0.12304120719674985, 0.4358676726639582}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.06020000018179417) {
                            memcpy(var52, __extension__ (const double[]){0.5285913528591353, 0.1603905160390516, 0.3110181311018131}, 3 * sizeof(double));
                        } else {
                            memcpy(var52, __extension__ (const double[]){0.4596774193548387, 0.0846774193548387, 0.45564516129032256}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.08229999989271164) {
                        if (input[3] <= -0.0831499993801117) {
                            memcpy(var52, __extension__ (const double[]){0.1794871794871795, 0.10256410256410256, 0.717948717948718}, 3 * sizeof(double));
                        } else {
                            memcpy(var52, __extension__ (const double[]){0.18181818181818182, 0.36363636363636365, 0.45454545454545453}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.37575000524520874) {
                            memcpy(var52, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var52, __extension__ (const double[]){0.4383720930232558, 0.12209302325581395, 0.43953488372093025}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var52, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var49, var52, 3, var48);
    double var53[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var53, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.06589999794960022) {
            if (input[1] <= 0.37880000472068787) {
                if (input[1] <= 0.09180000051856041) {
                    if (input[3] <= -0.4718499928712845) {
                        memcpy(var53, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.5032000094652176) {
                            memcpy(var53, __extension__ (const double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var53, __extension__ (const double[]){0.4495548961424332, 0.1444114737883284, 0.40603363006923837}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[0] <= -1.7999999523162842) {
                        memcpy(var53, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.007149999961256981) {
                            memcpy(var53, __extension__ (const double[]){0.4234135667396061, 0.09080962800875274, 0.48577680525164113}, 3 * sizeof(double));
                        } else {
                            memcpy(var53, __extension__ (const double[]){0.2682926829268293, 0.04878048780487805, 0.6829268292682927}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var53, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var53, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var48, var53, 3, var47);
    double var54[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var54, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.38450001180171967) {
            if (input[2] <= 0.06589999794960022) {
                if (input[3] <= -0.03099999949336052) {
                    if (input[0] <= 1.25) {
                        memcpy(var54, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.03910000063478947) {
                            memcpy(var54, __extension__ (const double[]){0.4479144385026738, 0.13090909090909092, 0.4211764705882353}, 3 * sizeof(double));
                        } else {
                            memcpy(var54, __extension__ (const double[]){0.25925925925925924, 0.09259259259259259, 0.6481481481481481}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.005750000011175871) {
                        if (input[3] <= -0.007500000298023224) {
                            memcpy(var54, __extension__ (const double[]){0.40441176470588236, 0.29411764705882354, 0.3014705882352941}, 3 * sizeof(double));
                        } else {
                            memcpy(var54, __extension__ (const double[]){0.1, 0.7, 0.2}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.0026000000070780516) {
                            memcpy(var54, __extension__ (const double[]){0.23529411764705882, 0.0, 0.7647058823529411}, 3 * sizeof(double));
                        } else {
                            memcpy(var54, __extension__ (const double[]){0.5140845070422535, 0.1619718309859155, 0.323943661971831}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var54, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var54, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var47, var54, 3, var46);
    double var55[3];
    if (input[0] <= 1.25) {
        memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= -0.023850000463426113) {
            if (input[1] <= -0.062049999833106995) {
                if (input[1] <= -0.062449999153614044) {
                    if (input[1] <= -0.3744499981403351) {
                        if (input[3] <= -0.2788500040769577) {
                            memcpy(var55, __extension__ (const double[]){0.25, 0.05, 0.7}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.4, 0.304, 0.296}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2560499906539917) {
                            memcpy(var55, __extension__ (const double[]){0.36807817589576547, 0.2280130293159609, 0.40390879478827363}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.4643557880967953, 0.14126880313930673, 0.394375408763898}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.06214999966323376) {
                        memcpy(var55, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= -0.12135000061243773) {
                            memcpy(var55, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.31575000286102295) {
                    if (input[3] <= -0.3250499963760376) {
                        if (input[1] <= 0.06224999949336052) {
                            memcpy(var55, __extension__ (const double[]){0.78125, 0.0, 0.21875}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.2, 0.06666666666666667, 0.7333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.32315000891685486) {
                            memcpy(var55, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.8, 0.0, 0.2}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.060599999502301216) {
                        if (input[1] <= -0.06104999966919422) {
                            memcpy(var55, __extension__ (const double[]){0.36666666666666664, 0.06666666666666667, 0.5666666666666667}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.02865000069141388) {
                            memcpy(var55, __extension__ (const double[]){0.43783783783783786, 0.12972972972972974, 0.43243243243243246}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.8461538461538461, 0.07692307692307693, 0.07692307692307693}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.010999999940395355) {
                if (input[2] <= -0.013849999755620956) {
                    if (input[1] <= 0.27025000751018524) {
                        if (input[2] <= -0.022199999541044235) {
                            memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.46, 0.12, 0.42}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.021700000390410423) {
                            memcpy(var55, __extension__ (const double[]){0.0, 0.2, 0.8}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.0, 0.8571428571428571, 0.14285714285714285}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.014949999283999205) {
                        if (input[3] <= -0.013100000098347664) {
                            memcpy(var55, __extension__ (const double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.8, 0.2, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[1] <= -0.3628000020980835) {
                    if (input[3] <= 0.04924999922513962) {
                        if (input[2] <= 0.02215000055730343) {
                            memcpy(var55, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[1] <= 0.14549999684095383) {
                        if (input[1] <= -0.35795000195503235) {
                            memcpy(var55, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.39669421487603307, 0.19834710743801653, 0.4049586776859504}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.2856000065803528) {
                            memcpy(var55, __extension__ (const double[]){0.09523809523809523, 0.09523809523809523, 0.8095238095238095}, 3 * sizeof(double));
                        } else {
                            memcpy(var55, __extension__ (const double[]){0.4117647058823529, 0.058823529411764705, 0.5294117647058824}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var46, var55, 3, var45);
    double var56[3];
    if (input[2] <= 0.06509999930858612) {
        if (input[1] <= -0.3846000134944916) {
            memcpy(var56, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[0] <= 1.25) {
                memcpy(var56, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[3] <= 0.049399999901652336) {
                    if (input[1] <= -0.17345000058412552) {
                        if (input[1] <= -0.18025000393390656) {
                            memcpy(var56, __extension__ (const double[]){0.4531986531986532, 0.1797979797979798, 0.367003367003367}, 3 * sizeof(double));
                        } else {
                            memcpy(var56, __extension__ (const double[]){0.8055555555555556, 0.16666666666666666, 0.027777777777777776}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.043950000777840614) {
                            memcpy(var56, __extension__ (const double[]){0.44188084112149534, 0.11711448598130841, 0.44100467289719625}, 3 * sizeof(double));
                        } else {
                            memcpy(var56, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 0.06039999984204769) {
                        if (input[3] <= 0.05454999953508377) {
                            memcpy(var56, __extension__ (const double[]){0.0, 0.5714285714285714, 0.42857142857142855}, 3 * sizeof(double));
                        } else {
                            memcpy(var56, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var56, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    } else {
        memcpy(var56, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var45, var56, 3, var44);
    double var57[3];
    if (input[0] <= 1.25) {
        memcpy(var57, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.09394999966025352) {
            if (input[3] <= -0.4011000096797943) {
                if (input[2] <= -0.4718499928712845) {
                    if (input[1] <= -0.1057500010356307) {
                        if (input[3] <= -0.5019499957561493) {
                            memcpy(var57, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var57, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var57, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[1] <= 0.09284999966621399) {
                    if (input[3] <= -0.33685000240802765) {
                        if (input[3] <= -0.33765000104904175) {
                            memcpy(var57, __extension__ (const double[]){0.13636363636363635, 0.22727272727272727, 0.6363636363636364}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.062449999153614044) {
                            memcpy(var57, __extension__ (const double[]){0.44307450157397693, 0.14428121720881426, 0.4126442812172088}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.6115702479338843, 0.09917355371900827, 0.2892561983471074}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var57, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[1] <= 0.1233999989926815) {
                if (input[3] <= -0.01529999982449226) {
                    if (input[3] <= -0.19280000030994415) {
                        if (input[2] <= -0.20629999786615372) {
                            memcpy(var57, __extension__ (const double[]){0.28125, 0.03125, 0.6875}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.1039000004529953) {
                            memcpy(var57, __extension__ (const double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.07407407407407407, 0.037037037037037035, 0.8888888888888888}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var57, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[1] <= 0.34025000035762787) {
                    if (input[3] <= -0.3185500055551529) {
                        if (input[1] <= 0.16575000435113907) {
                            memcpy(var57, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.045099999755620956) {
                            memcpy(var57, __extension__ (const double[]){0.5132075471698113, 0.08679245283018867, 0.4}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.28571428571428575, 0.163265306122449, 0.5510204081632654}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.3493500053882599) {
                        if (input[3] <= -0.2513500079512596) {
                            memcpy(var57, __extension__ (const double[]){0.6666666666666666, 0.0, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.10416666666666667, 0.10416666666666667, 0.7916666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.24335000663995743) {
                            memcpy(var57, __extension__ (const double[]){0.2708333333333333, 0.125, 0.6041666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var57, __extension__ (const double[]){0.4857142857142857, 0.08095238095238096, 0.43333333333333335}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var44, var57, 3, var43);
    double var58[3];
    if (input[0] <= 1.25) {
        memcpy(var58, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.059449998661875725) {
            if (input[1] <= 0.09905000030994415) {
                if (input[1] <= -0.37655000388622284) {
                    if (input[3] <= -0.15815000236034393) {
                        if (input[3] <= -0.29625000059604645) {
                            memcpy(var58, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.6, 0.1, 0.3}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.37665000557899475) {
                            memcpy(var58, __extension__ (const double[]){0.3333333333333333, 0.16666666666666666, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.0, 0.15384615384615385, 0.8461538461538461}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.3761499971151352) {
                        if (input[2] <= -0.23889999836683273) {
                            memcpy(var58, __extension__ (const double[]){0.2727272727272727, 0.36363636363636365, 0.36363636363636365}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.8055555555555556, 0.1111111111111111, 0.08333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.043950000777840614) {
                            memcpy(var58, __extension__ (const double[]){0.45849495733126455, 0.1448151021463667, 0.3966899405223688}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.0, 0.13333333333333333, 0.8666666666666667}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.22460000216960907) {
                    if (input[1] <= 0.37485000491142273) {
                        if (input[1] <= 0.3692000061273575) {
                            memcpy(var58, __extension__ (const double[]){0.3412322274881517, 0.052132701421800945, 0.6066350710900474}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.34782608695652173, 0.2608695652173913, 0.391304347826087}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.37610000371932983) {
                            memcpy(var58, __extension__ (const double[]){0.07692307692307693, 0.0, 0.9230769230769231}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.31414999067783356) {
                        if (input[1] <= 0.2820499986410141) {
                            memcpy(var58, __extension__ (const double[]){0.46285714285714286, 0.06857142857142857, 0.4685714285714286}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.28, 0.09333333333333334, 0.6266666666666667}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2045999988913536) {
                            memcpy(var58, __extension__ (const double[]){0.8181818181818182, 0.045454545454545456, 0.13636363636363635}, 3 * sizeof(double));
                        } else {
                            memcpy(var58, __extension__ (const double[]){0.49201277955271566, 0.07667731629392971, 0.43130990415335463}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[3] <= 0.06589999794960022) {
                memcpy(var58, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                memcpy(var58, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        }
    }
    add_vectors(var43, var58, 3, var42);
    double var59[3];
    if (input[3] <= 0.06499999947845936) {
        if (input[3] <= -0.036399999633431435) {
            if (input[3] <= -0.3897999972105026) {
                if (input[3] <= -0.41120000183582306) {
                    if (input[1] <= -0.14525000005960464) {
                        if (input[3] <= -0.5361999869346619) {
                            memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var59, __extension__ (const double[]){0.2857142857142857, 0.5714285714285714, 0.14285714285714285}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.4658999890089035) {
                            memcpy(var59, __extension__ (const double[]){0.25, 0.0, 0.75}, 3 * sizeof(double));
                        } else {
                            memcpy(var59, __extension__ (const double[]){0.875, 0.0, 0.125}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[2] <= -0.5577999800443649) {
                    memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[0] <= -0.9449999332427979) {
                        memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= 0.08705000206828117) {
                            memcpy(var59, __extension__ (const double[]){0.46476241324079015, 0.13347570742124934, 0.4017618793379605}, 3 * sizeof(double));
                        } else {
                            memcpy(var59, __extension__ (const double[]){0.4532293986636971, 0.07015590200445435, 0.4766146993318486}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[1] <= -0.5250499993562698) {
                memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[1] <= 0.3787499964237213) {
                    if (input[2] <= -0.03179999999701977) {
                        if (input[2] <= -0.031950000673532486) {
                            memcpy(var59, __extension__ (const double[]){0.14285714285714285, 0.2571428571428571, 0.6}, 3 * sizeof(double));
                        } else {
                            memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= -1.7999999523162842) {
                            memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var59, __extension__ (const double[]){0.45149253731343286, 0.11567164179104478, 0.43283582089552236}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        memcpy(var59, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var42, var59, 3, var41);
    double var60[3];
    if (input[0] <= 1.25) {
        memcpy(var60, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= -0.39695000648498535) {
            if (input[2] <= -0.4109500050544739) {
                if (input[3] <= -0.4658999890089035) {
                    if (input[1] <= -0.31084999442100525) {
                        if (input[1] <= -0.36434999108314514) {
                            memcpy(var60, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.48970000445842743) {
                            memcpy(var60, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var60, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var60, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[3] <= -0.014450000133365393) {
                if (input[1] <= -0.061650000512599945) {
                    if (input[1] <= -0.07175000011920929) {
                        if (input[1] <= -0.3367000073194504) {
                            memcpy(var60, __extension__ (const double[]){0.4844827586206897, 0.14655172413793102, 0.3689655172413793}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.4190064794816415, 0.1511879049676026, 0.4298056155507559}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.06234999932348728) {
                            memcpy(var60, __extension__ (const double[]){0.7413793103448276, 0.1206896551724138, 0.13793103448275862}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.5357142857142857, 0.03571428571428571, 0.42857142857142855}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.3250499963760376) {
                        if (input[3] <= -0.33240000903606415) {
                            memcpy(var60, __extension__ (const double[]){0.2916666666666667, 0.16666666666666666, 0.5416666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.18181818181818182, 0.0, 0.8181818181818182}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.05980000086128712) {
                            memcpy(var60, __extension__ (const double[]){0.16279069767441862, 0.09302325581395349, 0.7441860465116279}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.42615558060879366, 0.12100714017286734, 0.452837279218339}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.01155000040307641) {
                    if (input[1] <= -0.21060000360012054) {
                        memcpy(var60, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.012100000400096178) {
                            memcpy(var60, __extension__ (const double[]){0.8333333333333334, 0.16666666666666666, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.03150000050663948) {
                        if (input[1] <= 0.149849995970726) {
                            memcpy(var60, __extension__ (const double[]){0.5959595959595959, 0.08080808080808081, 0.32323232323232326}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.391304347826087, 0.043478260869565216, 0.5652173913043478}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.03724999912083149) {
                            memcpy(var60, __extension__ (const double[]){0.0, 0.38461538461538464, 0.6153846153846154}, 3 * sizeof(double));
                        } else {
                            memcpy(var60, __extension__ (const double[]){0.5416666666666666, 0.125, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var41, var60, 3, var40);
    double var61[3];
    if (input[2] <= 0.07375000044703484) {
        if (input[0] <= 1.25) {
            memcpy(var61, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= -0.31344999372959137) {
                if (input[3] <= -0.33685000240802765) {
                    if (input[3] <= -0.33754999935626984) {
                        if (input[3] <= -0.5361999869346619) {
                            memcpy(var61, __extension__ (const double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.4375, 0.16666666666666666, 0.3958333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var61, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= -0.31565000116825104) {
                        if (input[1] <= -0.1919500008225441) {
                            memcpy(var61, __extension__ (const double[]){0.4, 0.15, 0.45}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.8103448275862069, 0.06896551724137931, 0.1206896551724138}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.004549999721348286) {
                            memcpy(var61, __extension__ (const double[]){0.5, 0.125, 0.375}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.16666666666666666, 0.8333333333333334, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.08454999700188637) {
                    if (input[3] <= -0.10954999923706055) {
                        if (input[1] <= 0.3764500021934509) {
                            memcpy(var61, __extension__ (const double[]){0.44597364568081993, 0.12650073206442167, 0.42752562225475843}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.8461538461538461, 0.0, 0.15384615384615385}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.10785000026226044) {
                            memcpy(var61, __extension__ (const double[]){0.8571428571428571, 0.11428571428571428, 0.02857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.5263157894736842, 0.09382151029748284, 0.37986270022883295}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.08345000073313713) {
                        if (input[1] <= -0.21424999833106995) {
                            memcpy(var61, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.0, 0.1, 0.9}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.04105000011622906) {
                            memcpy(var61, __extension__ (const double[]){0.35406698564593303, 0.17942583732057416, 0.4665071770334928}, 3 * sizeof(double));
                        } else {
                            memcpy(var61, __extension__ (const double[]){0.448692152917505, 0.11267605633802817, 0.4386317907444668}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var61, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var40, var61, 3, var39);
    double var62[3];
    if (input[0] <= 1.25) {
        memcpy(var62, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= -0.37174999713897705) {
            if (input[1] <= 0.16299999877810478) {
                if (input[2] <= -0.5103500038385391) {
                    memcpy(var62, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= -0.17899999767541885) {
                        memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.4344000071287155) {
                            memcpy(var62, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var62, __extension__ (const double[]){0.8, 0.0, 0.2}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            }
        } else {
            if (input[2] <= 0.053549999371171) {
                if (input[3] <= -0.33844999969005585) {
                    if (input[3] <= -0.3592499941587448) {
                        memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= 0.02779999910853803) {
                            memcpy(var62, __extension__ (const double[]){0.4166666666666667, 0.16666666666666666, 0.4166666666666667}, 3 * sizeof(double));
                        } else {
                            memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.08720000088214874) {
                        if (input[1] <= 0.07155000045895576) {
                            memcpy(var62, __extension__ (const double[]){0.4529607792873622, 0.13329915406306075, 0.413740066649577}, 3 * sizeof(double));
                        } else {
                            memcpy(var62, __extension__ (const double[]){0.5961538461538461, 0.11538461538461539, 0.28846153846153844}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.33375000953674316) {
                            memcpy(var62, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var62, __extension__ (const double[]){0.4293085655314757, 0.09287925696594428, 0.47781217750258}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 0.06349999830126762) {
                    if (input[1] <= -0.010649999603629112) {
                        if (input[3] <= 0.05454999953508377) {
                            memcpy(var62, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var62, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var62, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var39, var62, 3, var38);
    double var63[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var63, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.06589999794960022) {
            if (input[2] <= -0.025349999777972698) {
                if (input[1] <= 0.20629999786615372) {
                    if (input[3] <= -0.07019999995827675) {
                        if (input[1] <= -0.36104999482631683) {
                            memcpy(var63, __extension__ (const double[]){0.5174418604651163, 0.16569767441860464, 0.3168604651162791}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.4499851588008311, 0.14633422380528346, 0.40368061739388544}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.06839999929070473) {
                            memcpy(var63, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.4, 0.12941176470588237, 0.47058823529411764}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.23820000141859055) {
                        if (input[2] <= -0.1217000000178814) {
                            memcpy(var63, __extension__ (const double[]){0.234375, 0.015625, 0.75}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.4090909090909091, 0.22727272727272727, 0.36363636363636365}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.24970000237226486) {
                            memcpy(var63, __extension__ (const double[]){0.6206896551724138, 0.20689655172413793, 0.1724137931034483}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.4090056285178236, 0.09193245778611632, 0.49906191369606}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 0.3844500035047531) {
                    if (input[1] <= -0.3127500116825104) {
                        if (input[1] <= -0.37530000507831573) {
                            memcpy(var63, __extension__ (const double[]){0.6, 0.0, 0.4}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.07692307692307693, 0.38461538461538464, 0.5384615384615384}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.014649999793618917) {
                            memcpy(var63, __extension__ (const double[]){0.2711864406779661, 0.4406779661016949, 0.288135593220339}, 3 * sizeof(double));
                        } else {
                            memcpy(var63, __extension__ (const double[]){0.5028248587570622, 0.1016949152542373, 0.3954802259887006}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var63, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var63, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var38, var63, 3, var37);
    double var64[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var64, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= 0.06589999794960022) {
            if (input[1] <= 0.37880000472068787) {
                if (input[3] <= -0.46015000343322754) {
                    if (input[1] <= -0.27709999680519104) {
                        memcpy(var64, __extension__ (const double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var64, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= 0.059449998661875725) {
                        if (input[3] <= 0.044849999248981476) {
                            memcpy(var64, __extension__ (const double[]){0.4494155582426441, 0.14248286981056027, 0.40810157194679564}, 3 * sizeof(double));
                        } else {
                            memcpy(var64, __extension__ (const double[]){0.15, 0.1, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var64, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var64, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var64, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var37, var64, 3, var36);
    double var65[3];
    if (input[2] <= 0.07295000180602074) {
        if (input[0] <= 1.25) {
            memcpy(var65, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[1] <= 0.05095000006258488) {
                if (input[2] <= 0.059449998661875725) {
                    if (input[1] <= -0.3733999878168106) {
                        if (input[2] <= -0.09120000153779984) {
                            memcpy(var65, __extension__ (const double[]){0.625, 0.09722222222222222, 0.2777777777777778}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.43333333333333335, 0.26666666666666666, 0.3}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.05460000038146973) {
                            memcpy(var65, __extension__ (const double[]){0.4502215657311669, 0.15273264401772527, 0.3970457902511078}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var65, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[3] <= -0.038750000298023224) {
                    if (input[2] <= -0.1485000029206276) {
                        if (input[3] <= -0.1513499990105629) {
                            memcpy(var65, __extension__ (const double[]){0.4158415841584158, 0.09476661951909476, 0.4893917963224894}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.08333333333333333, 0.0, 0.9166666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.1423499956727028) {
                            memcpy(var65, __extension__ (const double[]){0.5897435897435898, 0.28205128205128205, 0.1282051282051282}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.4283121597096189, 0.11796733212341198, 0.4537205081669691}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.025299999862909317) {
                        if (input[2] <= -0.032100001350045204) {
                            memcpy(var65, __extension__ (const double[]){0.15384615384615385, 0.15384615384615385, 0.6923076923076923}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.022549999877810478) {
                            memcpy(var65, __extension__ (const double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var65, __extension__ (const double[]){0.352112676056338, 0.14084507042253522, 0.5070422535211268}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var65, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var36, var65, 3, var35);
    double var66[3];
    if (input[2] <= 0.06589999794960022) {
        if (input[3] <= -0.02689999993890524) {
            if (input[2] <= -0.39490000903606415) {
                if (input[0] <= 1.25) {
                    memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.4109500050544739) {
                        if (input[2] <= -0.5918499976396561) {
                            memcpy(var66, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.625, 0.0, 0.375}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[1] <= 0.09394999966025352) {
                    if (input[1] <= -0.5547500103712082) {
                        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= -0.11464999988675117) {
                            memcpy(var66, __extension__ (const double[]){0.4411559888579387, 0.14484679665738162, 0.4139972144846797}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.3960703205791106, 0.19648397104446744, 0.4074457083764219}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.13585000485181808) {
                        if (input[3] <= -0.09715000167489052) {
                            memcpy(var66, __extension__ (const double[]){0.20481927710843373, 0.060240963855421686, 0.7349397590361446}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.6, 0.08, 0.32}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.15730000287294388) {
                            memcpy(var66, __extension__ (const double[]){0.7090909090909091, 0.07272727272727272, 0.21818181818181817}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.4332874828060523, 0.07840440165061899, 0.48830811554332876}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[3] <= -0.005400000140070915) {
                if (input[2] <= -0.0070500001311302185) {
                    if (input[0] <= -0.9449999332427979) {
                        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= -0.26684999465942383) {
                            memcpy(var66, __extension__ (const double[]){0.20833333333333334, 0.16666666666666666, 0.625}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.4375, 0.2375, 0.325}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.006449999986216426) {
                        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= -0.005949999904260039) {
                            memcpy(var66, __extension__ (const double[]){0.5, 0.25, 0.25}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.4, 0.6, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 0.00940000033006072) {
                    if (input[0] <= -1.7999999523162842) {
                        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= 0.006700000027194619) {
                            memcpy(var66, __extension__ (const double[]){0.32608695652173914, 0.043478260869565216, 0.6304347826086957}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.35795000195503235) {
                        if (input[1] <= -0.5347999930381775) {
                            memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.1, 0.7, 0.2}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.010250000283122063) {
                            memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var66, __extension__ (const double[]){0.5774647887323944, 0.056338028169014086, 0.36619718309859156}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var66, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var35, var66, 3, var34);
    double var67[3];
    if (input[2] <= 0.06589999794960022) {
        if (input[0] <= 1.25) {
            memcpy(var67, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= -0.015850000083446503) {
                if (input[2] <= -0.3642999976873398) {
                    if (input[1] <= 0.023250000551342964) {
                        if (input[1] <= -0.22154999524354935) {
                            memcpy(var67, __extension__ (const double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.875, 0.0, 0.125}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.37595000863075256) {
                            memcpy(var67, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.18685000389814377) {
                        if (input[1] <= 0.13555000722408295) {
                            memcpy(var67, __extension__ (const double[]){0.450229709035222, 0.14522715671260847, 0.4045431342521695}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.5932203389830508, 0.06779661016949153, 0.3389830508474576}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.020450000651180744) {
                            memcpy(var67, __extension__ (const double[]){0.432510885341074, 0.07402031930333818, 0.4934687953555878}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.011100000236183405) {
                    if (input[2] <= -0.01220000023022294) {
                        if (input[1] <= -0.2370000034570694) {
                            memcpy(var67, __extension__ (const double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.92, 0.04, 0.04}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.01155000040307641) {
                            memcpy(var67, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 0.008649999741464853) {
                        if (input[2] <= 0.005649999948218465) {
                            memcpy(var67, __extension__ (const double[]){0.5064935064935064, 0.07792207792207792, 0.4155844155844156}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.09090909090909091, 0.0, 0.9090909090909091}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.012600000016391277) {
                            memcpy(var67, __extension__ (const double[]){0.42857142857142855, 0.42857142857142855, 0.14285714285714285}, 3 * sizeof(double));
                        } else {
                            memcpy(var67, __extension__ (const double[]){0.6219512195121951, 0.07317073170731707, 0.3048780487804878}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var67, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var34, var67, 3, var33);
    double var68[3];
    if (input[2] <= 0.06509999930858612) {
        if (input[3] <= -0.02505000028759241) {
            if (input[3] <= -0.3948500007390976) {
                if (input[0] <= 1.0450000017881393) {
                    memcpy(var68, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.4109500050544739) {
                        if (input[1] <= 0.16299999877810478) {
                            memcpy(var68, __extension__ (const double[]){0.7647058823529411, 0.058823529411764705, 0.17647058823529413}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var68, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[3] <= -0.07795000076293945) {
                    if (input[2] <= -0.09710000082850456) {
                        if (input[1] <= 0.06039999984204769) {
                            memcpy(var68, __extension__ (const double[]){0.46304572017875556, 0.14162942591956, 0.3953248539016844}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.4135593220338983, 0.11073446327683616, 0.47570621468926555}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.3764500021934509) {
                            memcpy(var68, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.5168539325842697, 0.16292134831460675, 0.3202247191011236}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.02664999943226576) {
                        if (input[1] <= 0.34164999425411224) {
                            memcpy(var68, __extension__ (const double[]){0.3690248565965583, 0.17782026768642448, 0.45315487571701724}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.6666666666666666, 0.02564102564102564, 0.3076923076923077}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var68, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[3] <= -0.005300000077113509) {
                if (input[0] <= -0.9449999332427979) {
                    memcpy(var68, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= -0.26684999465942383) {
                        if (input[1] <= -0.3646000027656555) {
                            memcpy(var68, __extension__ (const double[]){0.6, 0.2, 0.2}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.14285714285714285, 0.0, 0.8571428571428571}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.006999999983236194) {
                            memcpy(var68, __extension__ (const double[]){0.5731707317073171, 0.20731707317073172, 0.21951219512195125}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.125, 0.75, 0.125}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 0.008750000037252903) {
                    if (input[3] <= 0.006700000027194619) {
                        if (input[3] <= -0.00020000000586151145) {
                            memcpy(var68, __extension__ (const double[]){0.2857142857142857, 0.08163265306122448, 0.6326530612244898}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.6190476190476191, 0.09523809523809523, 0.2857142857142857}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var68, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[3] <= 0.04254999943077564) {
                        if (input[1] <= -0.2866999953985214) {
                            memcpy(var68, __extension__ (const double[]){0.06451612903225806, 0.8709677419354839, 0.06451612903225806}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){0.5294117647058824, 0.13725490196078433, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.05869999900460243) {
                            memcpy(var68, __extension__ (const double[]){0.2, 0.2, 0.6}, 3 * sizeof(double));
                        } else {
                            memcpy(var68, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var68, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var33, var68, 3, var32);
    double var69[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var69, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= 0.06589999794960022) {
            if (input[0] <= 1.25) {
                memcpy(var69, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[2] <= -0.5103500038385391) {
                    memcpy(var69, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.022199999541044235) {
                        if (input[2] <= -0.03415000066161156) {
                            memcpy(var69, __extension__ (const double[]){0.4554391964094892, 0.1337892712117974, 0.4107715323787134}, 3 * sizeof(double));
                        } else {
                            memcpy(var69, __extension__ (const double[]){0.2676056338028169, 0.22535211267605634, 0.5070422535211268}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.28540000319480896) {
                            memcpy(var69, __extension__ (const double[]){0.3125, 0.1875, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var69, __extension__ (const double[]){0.5930232558139535, 0.09883720930232558, 0.3081395348837209}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var69, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var32, var69, 3, var31);
    double var70[3];
    if (input[3] <= 0.06589999794960022) {
        if (input[0] <= 1.25) {
            memcpy(var70, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[1] <= 0.08630000054836273) {
                if (input[1] <= 0.0852000005543232) {
                    if (input[1] <= 0.0770999975502491) {
                        if (input[2] <= -0.01500000013038516) {
                            memcpy(var70, __extension__ (const double[]){0.4379001280409731, 0.15185659411011523, 0.4102432778489116}, 3 * sizeof(double));
                        } else {
                            memcpy(var70, __extension__ (const double[]){0.5422535211267606, 0.16901408450704225, 0.2887323943661972}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.07954999804496765) {
                            memcpy(var70, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var70, __extension__ (const double[]){0.5833333333333334, 0.08333333333333333, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var70, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[2] <= -0.022499999962747097) {
                    if (input[2] <= -0.0442500002682209) {
                        if (input[3] <= -0.06104999966919422) {
                            memcpy(var70, __extension__ (const double[]){0.4411764705882353, 0.09647058823529411, 0.4623529411764706}, 3 * sizeof(double));
                        } else {
                            memcpy(var70, __extension__ (const double[]){0.7567567567567568, 0.0, 0.24324324324324326}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.042650001123547554) {
                            memcpy(var70, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var70, __extension__ (const double[]){0.07142857142857142, 0.14285714285714285, 0.7857142857142857}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.11024999991059303) {
                        memcpy(var70, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= -0.009850000031292439) {
                            memcpy(var70, __extension__ (const double[]){0.9285714285714286, 0.0, 0.07142857142857142}, 3 * sizeof(double));
                        } else {
                            memcpy(var70, __extension__ (const double[]){0.5517241379310345, 0.0, 0.4482758620689655}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var70, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var31, var70, 3, var30);
    double var71[3];
    if (input[0] <= 1.25) {
        memcpy(var71, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= -0.24889999628067017) {
            if (input[3] <= -0.2788500040769577) {
                if (input[3] <= -0.33375000953674316) {
                    if (input[2] <= -0.3352999985218048) {
                        if (input[1] <= 0.02319999970495701) {
                            memcpy(var71, __extension__ (const double[]){0.52, 0.24, 0.24}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.2, 0.15, 0.65}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.33455000817775726) {
                            memcpy(var71, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.25, 0.75, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.2807000130414963) {
                        if (input[1] <= -0.005850000074133277) {
                            memcpy(var71, __extension__ (const double[]){0.3869565217391304, 0.14347826086956522, 0.46956521739130436}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.526595744680851, 0.10106382978723404, 0.3723404255319149}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.280349999666214) {
                            memcpy(var71, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.47368421052631576, 0.0, 0.5263157894736842}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.27844999730587006) {
                    if (input[2] <= -0.27870000898838043) {
                        memcpy(var71, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= -0.14975000359117985) {
                            memcpy(var71, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.75, 0.25, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.3751000016927719) {
                        if (input[1] <= -0.3764999955892563) {
                            memcpy(var71, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.2222222222222222, 0.0, 0.7777777777777778}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2554499953985214) {
                            memcpy(var71, __extension__ (const double[]){0.5663082437275986, 0.15412186379928317, 0.27956989247311825}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.6938775510204082, 0.07142857142857142, 0.23469387755102042}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.2430500015616417) {
                if (input[2] <= -0.24594999849796295) {
                    if (input[2] <= -0.2476000040769577) {
                        if (input[1] <= -0.26224999874830246) {
                            memcpy(var71, __extension__ (const double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2471499964594841) {
                            memcpy(var71, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.4, 0.0, 0.6}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.055599999614059925) {
                        if (input[3] <= -0.24460000544786453) {
                            memcpy(var71, __extension__ (const double[]){0.0, 0.75, 0.25}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.45454545454545453, 0.18181818181818182, 0.36363636363636365}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.24524999409914017) {
                            memcpy(var71, __extension__ (const double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.0, 0.0625, 0.9375}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 0.009000000078231096) {
                    if (input[2] <= -0.24274999648332596) {
                        if (input[2] <= -0.24284999817609787) {
                            memcpy(var71, __extension__ (const double[]){0.625, 0.375, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.0018000000272877514) {
                            memcpy(var71, __extension__ (const double[]){0.44633642930856554, 0.13802889576883384, 0.41563467492260064}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.18181818181818182, 0.2727272727272727, 0.5454545454545454}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 0.031300000846385956) {
                        if (input[1] <= -0.21655000001192093) {
                            memcpy(var71, __extension__ (const double[]){0.375, 0.0, 0.625}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.8780487804878049, 0.024390243902439025, 0.0975609756097561}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.059449998661875725) {
                            memcpy(var71, __extension__ (const double[]){0.3235294117647059, 0.08823529411764706, 0.5882352941176471}, 3 * sizeof(double));
                        } else {
                            memcpy(var71, __extension__ (const double[]){0.8333333333333334, 0.16666666666666666, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var30, var71, 3, var29);
    double var72[3];
    if (input[0] <= 1.25) {
        memcpy(var72, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.04474999941885471) {
            if (input[3] <= -0.07519999891519547) {
                if (input[1] <= -0.3505000025033951) {
                    if (input[3] <= -0.17875000089406967) {
                        if (input[1] <= -0.3584499955177307) {
                            memcpy(var72, __extension__ (const double[]){0.4900662251655629, 0.2185430463576159, 0.2913907284768212}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.8857142857142857, 0.05714285714285714, 0.05714285714285714}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.36124999821186066) {
                            memcpy(var72, __extension__ (const double[]){0.4970414201183432, 0.11834319526627218, 0.38461538461538464}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.3469387755102041, 0.08163265306122448, 0.5714285714285714}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.08740000054240227) {
                        if (input[3] <= -0.13475000113248825) {
                            memcpy(var72, __extension__ (const double[]){0.4345137717818999, 0.13322091062394603, 0.43226531759415404}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.5286738351254481, 0.12365591397849462, 0.34767025089605735}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.04075000062584877) {
                            memcpy(var72, __extension__ (const double[]){0.3178294573643411, 0.10852713178294573, 0.5736434108527132}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.07225000113248825) {
                    if (input[2] <= -0.07490000128746033) {
                        if (input[2] <= -0.0750500001013279) {
                            memcpy(var72, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.07445000112056732) {
                            memcpy(var72, __extension__ (const double[]){0.0, 0.2, 0.8}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.05555555555555555, 0.5, 0.4444444444444444}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.03150000050663948) {
                        if (input[1] <= 0.03350000083446503) {
                            memcpy(var72, __extension__ (const double[]){0.47113163972286376, 0.15704387990762125, 0.371824480369515}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.5142857142857142, 0.34285714285714286, 0.14285714285714285}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.037299999967217445) {
                            memcpy(var72, __extension__ (const double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.375, 0.20833333333333334, 0.4166666666666667}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[3] <= -0.2024499997496605) {
                if (input[1] <= 0.053050000220537186) {
                    if (input[1] <= 0.05049999989569187) {
                        if (input[3] <= -0.23515000194311142) {
                            memcpy(var72, __extension__ (const double[]){0.5294117647058824, 0.17647058823529413, 0.29411764705882354}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.25, 0.0, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.05089999921619892) {
                            memcpy(var72, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.2, 0.0, 0.8}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.21005000174045563) {
                        if (input[3] <= -0.2107500061392784) {
                            memcpy(var72, __extension__ (const double[]){0.49870801033591733, 0.10594315245478036, 0.3953488372093023}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.20384999364614487) {
                            memcpy(var72, __extension__ (const double[]){0.8928571428571429, 0.07142857142857142, 0.03571428571428571}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.4444444444444444, 0.2222222222222222, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.1907999962568283) {
                    if (input[2] <= -0.1960499957203865) {
                        if (input[3] <= -0.19865000247955322) {
                            memcpy(var72, __extension__ (const double[]){0.3076923076923077, 0.05128205128205128, 0.6410256410256411}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.6666666666666666, 0.041666666666666664, 0.2916666666666667}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.1931999996304512) {
                            memcpy(var72, __extension__ (const double[]){0.0, 0.05555555555555555, 0.9444444444444444}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.20833333333333334, 0.0, 0.7916666666666666}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.10640000179409981) {
                        if (input[3] <= -0.1099500022828579) {
                            memcpy(var72, __extension__ (const double[]){0.4029038112522686, 0.12704174228675136, 0.47005444646098005}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.09005000069737434) {
                            memcpy(var72, __extension__ (const double[]){0.3020833333333333, 0.09375, 0.6041666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var72, __extension__ (const double[]){0.44375, 0.06875, 0.4875}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var29, var72, 3, var28);
    double var73[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var73, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[0] <= 1.25) {
            memcpy(var73, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[2] <= -0.2845499962568283) {
                if (input[3] <= -0.2848999947309494) {
                    if (input[3] <= -0.4109500050544739) {
                        if (input[3] <= -0.4658999890089035) {
                            memcpy(var73, __extension__ (const double[]){0.3333333333333333, 0.0, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.33470000326633453) {
                            memcpy(var73, __extension__ (const double[]){0.28, 0.18, 0.54}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){0.4748603351955307, 0.18994413407821228, 0.33519553072625696}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var73, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[2] <= -0.26545000076293945) {
                    if (input[1] <= 0.08035000041127205) {
                        if (input[2] <= -0.27924999594688416) {
                            memcpy(var73, __extension__ (const double[]){0.5142857142857142, 0.02857142857142857, 0.45714285714285713}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){0.28313253012048195, 0.1746987951807229, 0.5421686746987951}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.3479999899864197) {
                            memcpy(var73, __extension__ (const double[]){0.6410256410256411, 0.02564102564102564, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){0.2857142857142857, 0.07142857142857142, 0.6428571428571429}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.28154999017715454) {
                        if (input[2] <= -0.2649499922990799) {
                            memcpy(var73, __extension__ (const double[]){0.8, 0.06666666666666667, 0.13333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){0.46086282614311547, 0.134590545078791, 0.4045466287780935}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.25530000030994415) {
                            memcpy(var73, __extension__ (const double[]){0.7142857142857143, 0.0, 0.2857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var73, __extension__ (const double[]){0.41475826972010177, 0.09669211195928754, 0.48854961832061067}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var28, var73, 3, var27);
    double var74[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var74, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.39810000360012054) {
            if (input[2] <= 0.06509999930858612) {
                if (input[1] <= 0.08734999969601631) {
                    if (input[1] <= -0.37655000388622284) {
                        if (input[1] <= -0.37665000557899475) {
                            memcpy(var74, __extension__ (const double[]){0.4230769230769231, 0.07692307692307693, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var74, __extension__ (const double[]){0.13333333333333333, 0.06666666666666667, 0.8}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.3761499971151352) {
                            memcpy(var74, __extension__ (const double[]){0.7297297297297297, 0.21621621621621623, 0.05405405405405406}, 3 * sizeof(double));
                        } else {
                            memcpy(var74, __extension__ (const double[]){0.45346738850219126, 0.15622583139984533, 0.3903067800979634}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[0] <= -1.7999999523162842) {
                        memcpy(var74, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.32044999301433563) {
                            memcpy(var74, __extension__ (const double[]){0.14285714285714285, 0.047619047619047616, 0.8095238095238095}, 3 * sizeof(double));
                        } else {
                            memcpy(var74, __extension__ (const double[]){0.4611602753195674, 0.09046214355948869, 0.44837758112094395}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var74, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var74, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var27, var74, 3, var26);
    double var75[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var75, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.0632999986410141) {
            if (input[2] <= 0.053999999538064) {
                if (input[1] <= 0.37880000472068787) {
                    if (input[0] <= 1.25) {
                        memcpy(var75, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.6176999807357788) {
                            memcpy(var75, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var75, __extension__ (const double[]){0.4523905182804339, 0.13941341904379267, 0.4081960626757734}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var75, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var75, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var75, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var26, var75, 3, var25);
    double var76[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var76, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[0] <= 1.1349999979138374) {
            memcpy(var76, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[2] <= 0.044849999248981476) {
                if (input[1] <= 0.09180000051856041) {
                    if (input[3] <= 0.019100000150501728) {
                        if (input[2] <= -0.48979999125003815) {
                            memcpy(var76, __extension__ (const double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var76, __extension__ (const double[]){0.4504343382728666, 0.14690853346959631, 0.402657128257537}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.03155000042170286) {
                            memcpy(var76, __extension__ (const double[]){0.6666666666666666, 0.20833333333333334, 0.125}, 3 * sizeof(double));
                        } else {
                            memcpy(var76, __extension__ (const double[]){0.3181818181818182, 0.4090909090909091, 0.2727272727272727}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.3103499859571457) {
                        if (input[3] <= -0.2943999916315079) {
                            memcpy(var76, __extension__ (const double[]){0.2, 0.02857142857142857, 0.7714285714285715}, 3 * sizeof(double));
                        } else {
                            memcpy(var76, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2932500094175339) {
                            memcpy(var76, __extension__ (const double[]){0.6285714285714286, 0.11428571428571428, 0.2571428571428571}, 3 * sizeof(double));
                        } else {
                            memcpy(var76, __extension__ (const double[]){0.42857142857142855, 0.08748615725359911, 0.4839424141749723}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 0.31345000863075256) {
                    if (input[1] <= -0.02044999971985817) {
                        if (input[1] <= -0.08840000070631504) {
                            memcpy(var76, __extension__ (const double[]){0.125, 0.0, 0.875}, 3 * sizeof(double));
                        } else {
                            memcpy(var76, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var76, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[1] <= 0.3658500015735626) {
                        memcpy(var76, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var76, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    }
    add_vectors(var25, var76, 3, var24);
    double var77[3];
    if (input[0] <= 1.25) {
        memcpy(var77, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.05914999917149544) {
            if (input[3] <= 0.04959999956190586) {
                if (input[2] <= -0.29635000228881836) {
                    if (input[1] <= 0.0507499985396862) {
                        if (input[2] <= -0.32109999656677246) {
                            memcpy(var77, __extension__ (const double[]){0.45, 0.125, 0.425}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.5967741935483871, 0.20967741935483872, 0.1935483870967742}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.3185499906539917) {
                            memcpy(var77, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.03784999996423721) {
                        if (input[3] <= -0.13984999805688858) {
                            memcpy(var77, __extension__ (const double[]){0.4342607746863066, 0.1298417894162575, 0.4358974358974359}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.46497764530551416, 0.15052160953800298, 0.38450074515648286}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.03825000114738941) {
                            memcpy(var77, __extension__ (const double[]){0.5014749262536873, 0.1415929203539823, 0.35693215339233036}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.5882352941176471, 0.23529411764705882, 0.17647058823529413}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 0.06269999966025352) {
                    if (input[1] <= -0.360150009393692) {
                        memcpy(var77, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= 0.05454999953508377) {
                            memcpy(var77, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var77, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[3] <= 0.044449999928474426) {
                if (input[3] <= -0.3486499935388565) {
                    memcpy(var77, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.311599999666214) {
                        if (input[3] <= -0.31300000846385956) {
                            memcpy(var77, __extension__ (const double[]){0.5454545454545454, 0.22727272727272727, 0.22727272727272727}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.061249999329447746) {
                            memcpy(var77, __extension__ (const double[]){0.2727272727272727, 0.1038961038961039, 0.6233766233766234}, 3 * sizeof(double));
                        } else {
                            memcpy(var77, __extension__ (const double[]){0.445662100456621, 0.08949771689497717, 0.4648401826484018}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 0.06109999865293503) {
                    memcpy(var77, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    memcpy(var77, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var24, var77, 3, var23);
    double var78[3];
    if (input[2] <= 0.07589999958872795) {
        if (input[0] <= 1.25) {
            memcpy(var78, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= 0.058650000020861626) {
                if (input[2] <= -0.06430000066757202) {
                    if (input[1] <= -0.06234999932348728) {
                        if (input[3] <= -0.21719999611377716) {
                            memcpy(var78, __extension__ (const double[]){0.41986062717770034, 0.21428571428571427, 0.36585365853658536}, 3 * sizeof(double));
                        } else {
                            memcpy(var78, __extension__ (const double[]){0.4704579025110783, 0.1344165435745938, 0.3951255539143279}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.1347000002861023) {
                            memcpy(var78, __extension__ (const double[]){0.42254370102471367, 0.11573236889692586, 0.46172393007836043}, 3 * sizeof(double));
                        } else {
                            memcpy(var78, __extension__ (const double[]){0.468671679197995, 0.13533834586466167, 0.3959899749373434}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.05910000018775463) {
                        if (input[2] <= -0.06135000102221966) {
                            memcpy(var78, __extension__ (const double[]){0.36666666666666664, 0.03333333333333333, 0.6}, 3 * sizeof(double));
                        } else {
                            memcpy(var78, __extension__ (const double[]){0.09375, 0.09375, 0.8125}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.05784999951720238) {
                            memcpy(var78, __extension__ (const double[]){0.875, 0.0, 0.125}, 3 * sizeof(double));
                        } else {
                            memcpy(var78, __extension__ (const double[]){0.41767068273092367, 0.13453815261044177, 0.44779116465863456}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var78, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            }
        }
    } else {
        memcpy(var78, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var23, var78, 3, var22);
    double var79[3];
    if (input[0] <= 1.25) {
        memcpy(var79, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.09934999793767929) {
            if (input[3] <= 0.03155000042170286) {
                if (input[1] <= 0.04075000062584877) {
                    if (input[1] <= 0.039249999448657036) {
                        if (input[1] <= 0.0381499994546175) {
                            memcpy(var79, __extension__ (const double[]){0.4603078780561425, 0.13431934802293993, 0.4053727739209176}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.8235294117647058, 0.17647058823529413, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.16660000383853912) {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.04165000095963478) {
                        if (input[2] <= -0.13989999890327454) {
                            memcpy(var79, __extension__ (const double[]){0.8333333333333334, 0.16666666666666666, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.08000000193715096) {
                            memcpy(var79, __extension__ (const double[]){0.44274809160305345, 0.1946564885496183, 0.36259541984732824}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.6428571428571429, 0.08333333333333333, 0.27380952380952384}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 0.03500000014901161) {
                    if (input[2] <= 0.03395000100135803) {
                        if (input[1] <= -0.26235000789165497) {
                            memcpy(var79, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var79, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= 0.059449998661875725) {
                        if (input[2] <= 0.043950000777840614) {
                            memcpy(var79, __extension__ (const double[]){0.3, 0.1, 0.6}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.1111111111111111, 0.8888888888888888}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.16750000230967999) {
                            memcpy(var79, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.22540000081062317) {
                if (input[2] <= -0.2529499977827072) {
                    if (input[3] <= -0.3475499898195267) {
                        memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= 0.11335000023245811) {
                            memcpy(var79, __extension__ (const double[]){0.8888888888888888, 0.0, 0.1111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.48366013071895425, 0.0457516339869281, 0.47058823529411764}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.23405000567436218) {
                        if (input[2] <= -0.25015000253915787) {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0967741935483871, 0.22580645161290322, 0.6774193548387096}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.12584999948740005) {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.4, 0.05, 0.55}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.20179999619722366) {
                    if (input[2] <= -0.21005000174045563) {
                        if (input[1] <= 0.1184999980032444) {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.7073170731707318, 0.07317073170731708, 0.21951219512195125}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.20384999364614487) {
                            memcpy(var79, __extension__ (const double[]){0.9333333333333333, 0.06666666666666667, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.75, 0.25, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.10814999788999557) {
                        if (input[3] <= -0.1664000004529953) {
                            memcpy(var79, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.1711999997496605) {
                            memcpy(var79, __extension__ (const double[]){0.32, 0.07, 0.61}, 3 * sizeof(double));
                        } else {
                            memcpy(var79, __extension__ (const double[]){0.4777777777777778, 0.10925925925925926, 0.412962962962963}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var22, var79, 3, var21);
    double var80[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var80, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[2] <= 0.09474999830126762) {
            if (input[0] <= 1.25) {
                memcpy(var80, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[1] <= 0.09180000051856041) {
                    if (input[2] <= -0.37174999713897705) {
                        if (input[2] <= -0.5019499957561493) {
                            memcpy(var80, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var80, __extension__ (const double[]){0.75, 0.16666666666666666, 0.08333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.08864999935030937) {
                            memcpy(var80, __extension__ (const double[]){0.45443069931835395, 0.14365059328452412, 0.40191870739712193}, 3 * sizeof(double));
                        } else {
                            memcpy(var80, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.111200001090765) {
                        if (input[1] <= 0.11059999838471413) {
                            memcpy(var80, __extension__ (const double[]){0.16666666666666666, 0.14583333333333334, 0.6875}, 3 * sizeof(double));
                        } else {
                            memcpy(var80, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.05224999971687794) {
                            memcpy(var80, __extension__ (const double[]){0.44241119483315394, 0.0861141011840689, 0.4714747039827772}, 3 * sizeof(double));
                        } else {
                            memcpy(var80, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var80, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var21, var80, 3, var20);
    double var81[3];
    if (input[0] <= 1.25) {
        memcpy(var81, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= -0.12484999746084213) {
            if (input[2] <= -0.25874999165534973) {
                if (input[2] <= -0.2645000070333481) {
                    if (input[3] <= -0.2737499922513962) {
                        if (input[2] <= -0.2747500091791153) {
                            memcpy(var81, __extension__ (const double[]){0.4069767441860465, 0.21511627906976744, 0.37790697674418605}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2733500003814697) {
                            memcpy(var81, __extension__ (const double[]){0.0, 0.3076923076923077, 0.6923076923076923}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.3023255813953488, 0.18604651162790697, 0.5116279069767442}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.25984999537467957) {
                        if (input[2] <= -0.2603999972343445) {
                            memcpy(var81, __extension__ (const double[]){0.16666666666666666, 0.7222222222222222, 0.1111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.27274999022483826) {
                            memcpy(var81, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.2539999932050705) {
                    if (input[1] <= -0.35835000872612) {
                        if (input[3] <= -0.25679999589920044) {
                            memcpy(var81, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.3436499983072281) {
                            memcpy(var81, __extension__ (const double[]){0.75, 0.0, 0.25}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.9411764705882353, 0.0, 0.058823529411764705}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 0.037299999967217445) {
                        if (input[1] <= -0.1569499969482422) {
                            memcpy(var81, __extension__ (const double[]){0.45334370139968894, 0.14074650077760498, 0.4059097978227061}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.41007194244604317, 0.28776978417266186, 0.302158273381295}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.04959999956190586) {
                            memcpy(var81, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.6, 0.2, 0.2}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.31415000557899475) {
                if (input[2] <= -0.3251499980688095) {
                    if (input[2] <= -0.3291500061750412) {
                        if (input[1] <= 0.01919999998062849) {
                            memcpy(var81, __extension__ (const double[]){0.7777777777777778, 0.037037037037037035, 0.18518518518518517}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.5, 0.14285714285714285, 0.35714285714285715}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.327550008893013) {
                            memcpy(var81, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.5, 0.16666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.3185499906539917) {
                        if (input[1] <= -0.04059999994933605) {
                            memcpy(var81, __extension__ (const double[]){0.7142857142857143, 0.0, 0.2857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.04519999958574772) {
                            memcpy(var81, __extension__ (const double[]){0.875, 0.0, 0.125}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.3333333333333333, 0.0, 0.6666666666666666}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 0.06499999947845936) {
                    if (input[1] <= -0.11899999901652336) {
                        if (input[2] <= -0.20225000381469727) {
                            memcpy(var81, __extension__ (const double[]){0.6666666666666666, 0.0, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.047619047619047616, 0.09523809523809523, 0.8571428571428571}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.31300000846385956) {
                            memcpy(var81, __extension__ (const double[]){0.0, 0.16666666666666666, 0.8333333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var81, __extension__ (const double[]){0.44640564826700896, 0.11264441591784338, 0.44094993581514763}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var81, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var20, var81, 3, var19);
    double var82[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var82, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[0] <= 1.25) {
            memcpy(var82, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= -0.17864999920129776) {
                if (input[2] <= -0.18165000528097153) {
                    if (input[3] <= -0.22950000315904617) {
                        if (input[2] <= -0.24889999628067017) {
                            memcpy(var82, __extension__ (const double[]){0.47026431718061673, 0.1354625550660793, 0.394273127753304}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.3360323886639676, 0.17408906882591094, 0.4898785425101215}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.062449999153614044) {
                            memcpy(var82, __extension__ (const double[]){0.4626865671641791, 0.2064676616915423, 0.3308457711442786}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.5113636363636364, 0.08901515151515152, 0.3996212121212121}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.18150000274181366) {
                        memcpy(var82, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.18104999512434006) {
                            memcpy(var82, __extension__ (const double[]){0.8888888888888888, 0.0, 0.1111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.33333333333333337, 0.380952380952381, 0.28571428571428575}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.17555000633001328) {
                    if (input[2] <= -0.17785000056028366) {
                        if (input[3] <= -0.17844999581575394) {
                            memcpy(var82, __extension__ (const double[]){0.4, 0.0, 0.6}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.17695000022649765) {
                            memcpy(var82, __extension__ (const double[]){0.5454545454545454, 0.045454545454545456, 0.4090909090909091}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.17142857142857143, 0.0, 0.8285714285714286}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.13974999636411667) {
                        if (input[3] <= -0.01655000075697899) {
                            memcpy(var82, __extension__ (const double[]){0.43, 0.14666666666666667, 0.42333333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.5723684210526315, 0.13815789473684212, 0.2894736842105263}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.09075000137090683) {
                            memcpy(var82, __extension__ (const double[]){0.46864686468646866, 0.11221122112211221, 0.41914191419141916}, 3 * sizeof(double));
                        } else {
                            memcpy(var82, __extension__ (const double[]){0.3681592039800995, 0.04975124378109453, 0.582089552238806}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var19, var82, 3, var18);
    double var83[3];
    if (input[3] <= 0.06589999794960022) {
        if (input[1] <= -0.3846000134944916) {
            memcpy(var83, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[2] <= -0.030749999918043613) {
                if (input[3] <= -0.035749999806284904) {
                    if (input[1] <= 0.12885000556707382) {
                        if (input[0] <= 1.25) {
                            memcpy(var83, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.4318181818181818, 0.14707419017763845, 0.4211076280041797}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.33994999527931213) {
                            memcpy(var83, __extension__ (const double[]){0.5170556552962298, 0.06822262118491922, 0.414721723518851}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.4070175438596491, 0.13333333333333333, 0.45964912280701753}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.030849999748170376) {
                        if (input[3] <= -0.0345000009983778) {
                            memcpy(var83, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.2, 0.05, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var83, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[3] <= -0.004449999891221523) {
                    if (input[3] <= -0.013849999755620956) {
                        if (input[3] <= -0.019899999722838402) {
                            memcpy(var83, __extension__ (const double[]){0.5094339622641509, 0.22641509433962265, 0.2641509433962264}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.18181818181818182, 0.41818181818181815, 0.4}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.010999999940395355) {
                            memcpy(var83, __extension__ (const double[]){0.5714285714285714, 0.42857142857142855, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.45454545454545453, 0.2727272727272727, 0.2727272727272727}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 0.008850000333040953) {
                        if (input[2] <= 0.0023999999975785613) {
                            memcpy(var83, __extension__ (const double[]){0.40540540540540543, 0.10810810810810811, 0.4864864864864865}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.058823529411764705, 0.11764705882352941, 0.8235294117647058}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.03265000134706497) {
                            memcpy(var83, __extension__ (const double[]){0.5964912280701754, 0.17543859649122806, 0.22807017543859648}, 3 * sizeof(double));
                        } else {
                            memcpy(var83, __extension__ (const double[]){0.34782608695652173, 0.10869565217391304, 0.5434782608695652}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var83, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var18, var83, 3, var17);
    double var84[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var84, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.37880000472068787) {
            if (input[2] <= 0.06509999930858612) {
                if (input[0] <= 1.25) {
                    memcpy(var84, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[3] <= -0.3278000056743622) {
                        if (input[3] <= -0.330949991941452) {
                            memcpy(var84, __extension__ (const double[]){0.41333333333333333, 0.08, 0.5066666666666667}, 3 * sizeof(double));
                        } else {
                            memcpy(var84, __extension__ (const double[]){0.058823529411764705, 0.058823529411764705, 0.8823529411764706}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.3110499978065491) {
                            memcpy(var84, __extension__ (const double[]){0.6144578313253012, 0.060240963855421686, 0.3253012048192771}, 3 * sizeof(double));
                        } else {
                            memcpy(var84, __extension__ (const double[]){0.45877466251298027, 0.1395638629283489, 0.4016614745586708}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var84, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var84, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var17, var84, 3, var16);
    double var85[3];
    if (input[0] <= 1.25) {
        memcpy(var85, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= -0.2470499947667122) {
            if (input[1] <= -0.17970000207424164) {
                if (input[2] <= -0.26524999737739563) {
                    if (input[3] <= -0.3279000073671341) {
                        if (input[3] <= -0.34939999878406525) {
                            memcpy(var85, __extension__ (const double[]){0.3333333333333333, 0.5555555555555556, 0.1111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.0, 0.46153846153846156, 0.5384615384615384}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.2966499924659729) {
                            memcpy(var85, __extension__ (const double[]){0.5238095238095238, 0.19047619047619047, 0.2857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.29508196721311475, 0.319672131147541, 0.38524590163934425}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.37074999511241913) {
                        if (input[2] <= -0.2627999931573868) {
                            memcpy(var85, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.2539999932050705) {
                            memcpy(var85, __extension__ (const double[]){0.7804878048780488, 0.0975609756097561, 0.12195121951219512}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.5185185185185185, 0.2222222222222222, 0.25925925925925924}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= -0.25039999186992645) {
                    if (input[3] <= -0.45419999957084656) {
                        memcpy(var85, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= -0.16870000213384628) {
                            memcpy(var85, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.4812925170068027, 0.11054421768707483, 0.40816326530612246}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.24889999628067017) {
                        memcpy(var85, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.24814999848604202) {
                            memcpy(var85, __extension__ (const double[]){0.0, 0.16666666666666666, 0.8333333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[1] <= 0.06224999949336052) {
                if (input[2] <= -0.245449997484684) {
                    if (input[3] <= -0.24665000289678574) {
                        if (input[3] <= -0.2468000054359436) {
                            memcpy(var85, __extension__ (const double[]){0.14285714285714285, 0.0, 0.8571428571428571}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var85, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[1] <= -0.31735000014305115) {
                        if (input[2] <= -0.24424999952316284) {
                            memcpy(var85, __extension__ (const double[]){0.0, 0.2222222222222222, 0.7777777777777778}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.49335548172757476, 0.12956810631229235, 0.3770764119601329}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.13464999943971634) {
                            memcpy(var85, __extension__ (const double[]){0.39879608728367194, 0.16930022573363432, 0.43190368698269377}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.460840890354493, 0.16240725474031328, 0.37675185490519375}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.2430500015616417) {
                    memcpy(var85, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[3] <= -0.20270000398159027) {
                        if (input[3] <= -0.22020000219345093) {
                            memcpy(var85, __extension__ (const double[]){0.4107142857142857, 0.03571428571428571, 0.5535714285714286}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.7272727272727273, 0.045454545454545456, 0.22727272727272727}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.37164999544620514) {
                            memcpy(var85, __extension__ (const double[]){0.43490701001430615, 0.09728183118741059, 0.4678111587982833}, 3 * sizeof(double));
                        } else {
                            memcpy(var85, __extension__ (const double[]){0.3333333333333333, 0.05128205128205128, 0.6153846153846154}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var16, var85, 3, var15);
    double var86[3];
    if (input[3] <= 0.07375000044703484) {
        if (input[2] <= -0.3948500007390976) {
            if (input[1] <= -0.14525000005960464) {
                if (input[0] <= 1.25) {
                    memcpy(var86, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= -0.27709999680519104) {
                        memcpy(var86, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var86, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var86, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[3] <= -0.031150000169873238) {
                if (input[0] <= 0.75) {
                    memcpy(var86, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 0.06720000132918358) {
                        if (input[2] <= -0.08505000174045563) {
                            memcpy(var86, __extension__ (const double[]){0.452731737262124, 0.14211172498465316, 0.40515653775322286}, 3 * sizeof(double));
                        } else {
                            memcpy(var86, __extension__ (const double[]){0.35787321063394684, 0.1492842535787321, 0.49284253578732107}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.04019999876618385) {
                            memcpy(var86, __extension__ (const double[]){0.4610458911419424, 0.0736392742796158, 0.46531483457844186}, 3 * sizeof(double));
                        } else {
                            memcpy(var86, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= -0.5292499959468842) {
                    memcpy(var86, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.005300000077113509) {
                        if (input[3] <= -0.019899999722838402) {
                            memcpy(var86, __extension__ (const double[]){0.4727272727272727, 0.21818181818181817, 0.3090909090909091}, 3 * sizeof(double));
                        } else {
                            memcpy(var86, __extension__ (const double[]){0.308411214953271, 0.4953271028037383, 0.19626168224299065}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.000750000006519258) {
                            memcpy(var86, __extension__ (const double[]){0.3225806451612903, 0.06451612903225806, 0.6129032258064516}, 3 * sizeof(double));
                        } else {
                            memcpy(var86, __extension__ (const double[]){0.4260869565217391, 0.21739130434782608, 0.3565217391304348}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var86, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var15, var86, 3, var14);
    double var87[3];
    if (input[0] <= 1.25) {
        memcpy(var87, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= -0.1805500015616417) {
            if (input[2] <= -0.21644999831914902) {
                if (input[3] <= -0.21894999593496323) {
                    if (input[1] <= -0.3641500025987625) {
                        if (input[2] <= -0.26455000042915344) {
                            memcpy(var87, __extension__ (const double[]){0.38095238095238093, 0.09523809523809523, 0.5238095238095238}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.3404255319148936, 0.425531914893617, 0.23404255319148937}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.22015000134706497) {
                            memcpy(var87, __extension__ (const double[]){0.45819112627986347, 0.12286689419795221, 0.4189419795221843}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.8571428571428572, 0.07142857142857144, 0.07142857142857144}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.21819999814033508) {
                        if (input[3] <= -0.21845000237226486) {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.21694999933242798) {
                            memcpy(var87, __extension__ (const double[]){0.47368421052631576, 0.10526315789473684, 0.42105263157894735}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.1, 0.0, 0.9}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 0.2147500067949295) {
                    if (input[1] <= -0.06109999865293503) {
                        if (input[3] <= -0.18734999746084213) {
                            memcpy(var87, __extension__ (const double[]){0.542713567839196, 0.16080402010050251, 0.2964824120603015}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.5208333333333334, 0.3125, 0.16666666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.21549999713897705) {
                            memcpy(var87, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.48717948717948717, 0.10989010989010989, 0.40293040293040294}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.19770000129938126) {
                        if (input[1] <= 0.2989500015974045) {
                            memcpy(var87, __extension__ (const double[]){0.25, 0.16666666666666666, 0.5833333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.6666666666666666, 0.18518518518518517, 0.14814814814814814}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.37379999458789825) {
                            memcpy(var87, __extension__ (const double[]){0.15151515151515152, 0.0, 0.8484848484848485}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.17214999347925186) {
                if (input[2] <= -0.17265000194311142) {
                    if (input[1] <= -0.12794999778270721) {
                        if (input[3] <= -0.17989999800920486) {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.5263157894736842, 0.07017543859649122, 0.40350877192982454}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.17294999957084656) {
                            memcpy(var87, __extension__ (const double[]){0.29464285714285715, 0.07142857142857142, 0.6339285714285714}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.1724499985575676) {
                        memcpy(var87, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= 0.08394999988377094) {
                            memcpy(var87, __extension__ (const double[]){0.25, 0.0, 0.75}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.6, 0.4}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 0.06020000018179417) {
                    if (input[2] <= 0.044849999248981476) {
                        if (input[3] <= -0.17185000330209732) {
                            memcpy(var87, __extension__ (const double[]){0.7692307692307693, 0.07692307692307693, 0.15384615384615385}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.4343649946638207, 0.12913553895410887, 0.43649946638207043}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.04704999923706055) {
                            memcpy(var87, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var87, __extension__ (const double[]){0.14285714285714285, 0.14285714285714285, 0.7142857142857143}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.06589999794960022) {
                        memcpy(var87, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var87, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    }
    add_vectors(var14, var87, 3, var13);
    double var88[3];
    if (input[0] <= 1.25) {
        memcpy(var88, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= -0.39695000648498535) {
            if (input[2] <= -0.5103500038385391) {
                memcpy(var88, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[2] <= -0.4718499928712845) {
                    if (input[1] <= -0.23454999923706055) {
                        memcpy(var88, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var88, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= -0.4109500050544739) {
                        if (input[3] <= -0.44609999656677246) {
                            memcpy(var88, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.75, 0.0, 0.25}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var88, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[1] <= 0.0628499984741211) {
                if (input[3] <= -0.131400004029274) {
                    if (input[3] <= -0.16465000063180923) {
                        if (input[3] <= -0.16565000265836716) {
                            memcpy(var88, __extension__ (const double[]){0.43843683083511775, 0.14989293361884368, 0.41167023554603854}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.8421052631578947, 0.0, 0.15789473684210525}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.06070000119507313) {
                            memcpy(var88, __extension__ (const double[]){0.3639344262295082, 0.15737704918032788, 0.4786885245901639}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.6666666666666667, 0.2380952380952381, 0.09523809523809525}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.12354999780654907) {
                        if (input[1] <= -0.0511499997228384) {
                            memcpy(var88, __extension__ (const double[]){0.5319148936170213, 0.11702127659574468, 0.35106382978723405}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.7777777777777778, 0.05555555555555555, 0.16666666666666666}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.3414499908685684) {
                            memcpy(var88, __extension__ (const double[]){0.359375, 0.21354166666666666, 0.4270833333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.47354497354497355, 0.14638447971781304, 0.3800705467372134}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.258200004696846) {
                    if (input[2] <= -0.3166999965906143) {
                        if (input[2] <= -0.3642999976873398) {
                            memcpy(var88, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.2222222222222222, 0.05555555555555555, 0.7222222222222222}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.2691500037908554) {
                            memcpy(var88, __extension__ (const double[]){0.6063829787234043, 0.07446808510638298, 0.3191489361702128}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.9375, 0.0, 0.0625}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.2430500015616417) {
                        if (input[2] <= -0.24469999969005585) {
                            memcpy(var88, __extension__ (const double[]){0.20689655172413793, 0.06896551724137931, 0.7241379310344828}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.24184999614953995) {
                            memcpy(var88, __extension__ (const double[]){0.1, 0.8, 0.1}, 3 * sizeof(double));
                        } else {
                            memcpy(var88, __extension__ (const double[]){0.4231242312423124, 0.08856088560885608, 0.4883148831488315}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var13, var88, 3, var12);
    double var89[3];
    if (input[1] <= -0.3846000134944916) {
        memcpy(var89, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.06589999794960022) {
            if (input[2] <= -0.024050000123679638) {
                if (input[0] <= -2.299999952316284) {
                    memcpy(var89, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[3] <= -0.027699999511241913) {
                        if (input[3] <= -0.39695000648498535) {
                            memcpy(var89, __extension__ (const double[]){0.5555555555555556, 0.3888888888888889, 0.05555555555555555}, 3 * sizeof(double));
                        } else {
                            memcpy(var89, __extension__ (const double[]){0.4439966058549003, 0.1347051336444633, 0.4212982605006364}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.09105000272393227) {
                            memcpy(var89, __extension__ (const double[]){0.8333333333333334, 0.0, 0.16666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var89, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 0.3787499964237213) {
                    if (input[1] <= -0.29884999990463257) {
                        if (input[2] <= 0.05700000002980232) {
                            memcpy(var89, __extension__ (const double[]){0.15217391304347827, 0.32608695652173914, 0.5217391304347826}, 3 * sizeof(double));
                        } else {
                            memcpy(var89, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= -1.7999999523162842) {
                            memcpy(var89, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var89, __extension__ (const double[]){0.4449760765550239, 0.10047846889952153, 0.45454545454545453}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var89, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var89, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var12, var89, 3, var11);
    double var90[3];
    if (input[0] <= 1.25) {
        memcpy(var90, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= -0.06234999932348728) {
            if (input[3] <= -0.061400000005960464) {
                if (input[1] <= -0.3755500018596649) {
                    if (input[2] <= -0.07304999977350235) {
                        if (input[1] <= -0.376350000500679) {
                            memcpy(var90, __extension__ (const double[]){0.21428571428571427, 0.07142857142857142, 0.7142857142857143}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.2894736842105263, 0.21052631578947367, 0.5}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var90, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[3] <= -0.06264999881386757) {
                        if (input[1] <= -0.07200000062584877) {
                            memcpy(var90, __extension__ (const double[]){0.4639666468135795, 0.17450863609291245, 0.36152471709350803}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.6951219512195121, 0.06097560975609756, 0.24390243902439024}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var90, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[2] <= -0.054349999874830246) {
                    if (input[3] <= -0.05855000019073486) {
                        memcpy(var90, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= -0.2954999953508377) {
                            memcpy(var90, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.15384615384615385, 0.0, 0.8461538461538461}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.17390000075101852) {
                        if (input[3] <= 0.04959999956190586) {
                            memcpy(var90, __extension__ (const double[]){0.5514705882352942, 0.0661764705882353, 0.38235294117647056}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.2, 0.6, 0.2}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.046950001269578934) {
                            memcpy(var90, __extension__ (const double[]){0.5555555555555556, 0.3333333333333333, 0.1111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.3404255319148936, 0.02127659574468085, 0.6382978723404256}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[3] <= -0.07644999772310257) {
                if (input[3] <= -0.07824999839067459) {
                    if (input[1] <= -0.055800000205636024) {
                        if (input[2] <= -0.29180000722408295) {
                            memcpy(var90, __extension__ (const double[]){0.7142857142857143, 0.2857142857142857, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.3474576271186441, 0.025423728813559324, 0.6271186440677966}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.07910000160336494) {
                            memcpy(var90, __extension__ (const double[]){0.44871794871794873, 0.1220159151193634, 0.4292661361626879}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.0, 0.09090909090909091, 0.9090909090909091}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.0771000012755394) {
                        if (input[3] <= -0.07779999822378159) {
                            memcpy(var90, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.875, 0.0, 0.125}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.1825999985449016) {
                            memcpy(var90, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.06104999966919422) {
                    if (input[3] <= -0.07225000113248825) {
                        if (input[3] <= -0.07559999823570251) {
                            memcpy(var90, __extension__ (const double[]){0.06666666666666667, 0.06666666666666667, 0.8666666666666667}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.34285714285714286, 0.02857142857142857, 0.6285714285714286}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.001349999976810068) {
                            memcpy(var90, __extension__ (const double[]){0.6, 0.05, 0.35}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.19402985074626866, 0.26865671641791045, 0.5373134328358209}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.05375000089406967) {
                        if (input[3] <= -0.05625000037252903) {
                            memcpy(var90, __extension__ (const double[]){0.5777777777777777, 0.1111111111111111, 0.3111111111111111}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.8461538461538461, 0.0, 0.15384615384615385}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.0475000012665987) {
                            memcpy(var90, __extension__ (const double[]){0.15789473684210525, 0.3684210526315789, 0.47368421052631576}, 3 * sizeof(double));
                        } else {
                            memcpy(var90, __extension__ (const double[]){0.4141791044776119, 0.08955223880597014, 0.4962686567164179}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var11, var90, 3, var10);
    double var91[3];
    if (input[0] <= 1.25) {
        memcpy(var91, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.0628499984741211) {
            if (input[1] <= 0.062449999153614044) {
                if (input[1] <= 0.062049999833106995) {
                    if (input[3] <= 0.04924999922513962) {
                        if (input[3] <= -0.0613000001758337) {
                            memcpy(var91, __extension__ (const double[]){0.4628242074927954, 0.14063400576368876, 0.39654178674351587}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.4235588972431078, 0.11027568922305765, 0.4661654135338346}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.05570000037550926) {
                            memcpy(var91, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.0, 0.25, 0.75}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.18685000389814377) {
                        if (input[1] <= 0.06234999932348728) {
                            memcpy(var91, __extension__ (const double[]){0.5714285714285714, 0.0, 0.42857142857142855}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.25, 0.0, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.16110000014305115) {
                            memcpy(var91, __extension__ (const double[]){0.0, 0.6, 0.4}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.0, 0.13333333333333333, 0.8666666666666667}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.04349999956320971) {
                    if (input[2] <= -0.17764999717473984) {
                        if (input[2] <= -0.19564999639987946) {
                            memcpy(var91, __extension__ (const double[]){0.7272727272727273, 0.18181818181818182, 0.09090909090909091}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.1213500015437603) {
                            memcpy(var91, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.8, 0.2, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var91, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[3] <= -0.22749999910593033) {
                if (input[1] <= 0.32634998857975006) {
                    if (input[2] <= -0.23279999941587448) {
                        if (input[3] <= -0.24689999967813492) {
                            memcpy(var91, __extension__ (const double[]){0.3482142857142857, 0.044642857142857144, 0.6071428571428571}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.06896551724137931, 0.0, 0.9310344827586207}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.23240000009536743) {
                            memcpy(var91, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.6153846153846154, 0.0, 0.38461538461538464}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.2336999997496605) {
                        if (input[1] <= 0.3541499972343445) {
                            memcpy(var91, __extension__ (const double[]){0.8518518518518519, 0.0, 0.14814814814814814}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.3181818181818182, 0.25, 0.4318181818181818}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var91, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[3] <= -0.18549999594688416) {
                    if (input[1] <= 0.11584999784827232) {
                        if (input[2] <= -0.20899999886751175) {
                            memcpy(var91, __extension__ (const double[]){0.16666666666666666, 0.0, 0.8333333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.4666666666666667, 0.0, 0.5333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.192050002515316) {
                            memcpy(var91, __extension__ (const double[]){0.5867768595041323, 0.11570247933884298, 0.2975206611570248}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.8076923076923077, 0.038461538461538464, 0.15384615384615385}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.16979999840259552) {
                        if (input[2] <= -0.17500000447034836) {
                            memcpy(var91, __extension__ (const double[]){0.4761904761904762, 0.04761904761904762, 0.4761904761904762}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.1388888888888889, 0.027777777777777776, 0.8333333333333334}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= 0.01285000005736947) {
                            memcpy(var91, __extension__ (const double[]){0.4521276595744681, 0.08156028368794327, 0.46631205673758863}, 3 * sizeof(double));
                        } else {
                            memcpy(var91, __extension__ (const double[]){0.8, 0.0, 0.2}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var10, var91, 3, var9);
    double var92[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var92, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.37880000472068787) {
            if (input[3] <= 0.06589999794960022) {
                if (input[0] <= 1.25) {
                    memcpy(var92, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 0.058650000020861626) {
                        if (input[1] <= -0.3687500059604645) {
                            memcpy(var92, __extension__ (const double[]){0.5055350553505535, 0.17343173431734318, 0.3210332103321033}, 3 * sizeof(double));
                        } else {
                            memcpy(var92, __extension__ (const double[]){0.43952546296296297, 0.15335648148148148, 0.4071180555555556}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.3452000021934509) {
                            memcpy(var92, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var92, __extension__ (const double[]){0.46226415094339623, 0.08018867924528301, 0.45754716981132076}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var92, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var92, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var9, var92, 3, var8);
    double var93[3];
    if (input[2] <= 0.06589999794960022) {
        if (input[0] <= 1.25) {
            memcpy(var93, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= -0.52394999563694) {
                memcpy(var93, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[3] <= 0.06039999984204769) {
                    if (input[2] <= 0.04454999975860119) {
                        if (input[1] <= 0.08639999851584435) {
                            memcpy(var93, __extension__ (const double[]){0.4457648546144121, 0.13552465233881164, 0.4187104930467762}, 3 * sizeof(double));
                        } else {
                            memcpy(var93, __extension__ (const double[]){0.4441117764471058, 0.09181636726546906, 0.46407185628742514}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.04704999923706055) {
                            memcpy(var93, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var93, __extension__ (const double[]){0.2857142857142857, 0.0, 0.7142857142857143}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var93, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        memcpy(var93, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var8, var93, 3, var7);
    double var94[3];
    if (input[0] <= 1.25) {
        memcpy(var94, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= -0.1855499967932701) {
            if (input[2] <= -0.18734999746084213) {
                if (input[1] <= -0.14105000346899033) {
                    if (input[1] <= -0.14639999717473984) {
                        if (input[2] <= -0.31630000472068787) {
                            memcpy(var94, __extension__ (const double[]){0.10810810810810811, 0.16216216216216217, 0.7297297297297297}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.4431818181818182, 0.17045454545454544, 0.38636363636363635}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.25575000792741776) {
                            memcpy(var94, __extension__ (const double[]){0.3333333333333333, 0.3333333333333333, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.4109500050544739) {
                        memcpy(var94, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[1] <= -0.06234999932348728) {
                            memcpy(var94, __extension__ (const double[]){0.60431654676259, 0.1366906474820144, 0.2589928057553957}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.47780925401322, 0.09537299338999056, 0.42681775259678945}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.187049999833107) {
                    if (input[2] <= -0.18724999576807022) {
                        memcpy(var94, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.1871499940752983) {
                            memcpy(var94, __extension__ (const double[]){0.6, 0.4, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.1868000030517578) {
                        if (input[1] <= 0.026449999306350946) {
                            memcpy(var94, __extension__ (const double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.01504999974713428) {
                            memcpy(var94, __extension__ (const double[]){0.9375, 0.0625, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.6, 0.2, 0.2}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[2] <= -0.16774999350309372) {
                if (input[1] <= -0.37630000710487366) {
                    memcpy(var94, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[3] <= -0.17080000042915344) {
                        if (input[3] <= -0.1724499985575676) {
                            memcpy(var94, __extension__ (const double[]){0.359504132231405, 0.09917355371900827, 0.5413223140495868}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.6444444444444445, 0.044444444444444446, 0.3111111111111111}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.16905000060796738) {
                            memcpy(var94, __extension__ (const double[]){0.1276595744680851, 0.0851063829787234, 0.7872340425531915}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.28205128205128205, 0.1282051282051282, 0.5897435897435898}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= -0.1659500002861023) {
                    if (input[2] <= -0.16704999655485153) {
                        if (input[2] <= -0.16724999994039536) {
                            memcpy(var94, __extension__ (const double[]){0.6363636363636364, 0.0, 0.36363636363636365}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.05835000053048134) {
                            memcpy(var94, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.09874999895691872) {
                        if (input[2] <= -0.044999999925494194) {
                            memcpy(var94, __extension__ (const double[]){0.4292709466811752, 0.1528835690968444, 0.4178454842219804}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.4984126984126984, 0.1619047619047619, 0.3396825396825397}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.13484999537467957) {
                            memcpy(var94, __extension__ (const double[]){0.32867132867132864, 0.16783216783216784, 0.5034965034965035}, 3 * sizeof(double));
                        } else {
                            memcpy(var94, __extension__ (const double[]){0.4489247311827957, 0.053763440860215055, 0.49731182795698925}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var7, var94, 3, var6);
    double var95[3];
    if (input[0] <= 1.25) {
        memcpy(var95, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.0628499984741211) {
            if (input[3] <= 0.043950000777840614) {
                if (input[1] <= 0.062449999153614044) {
                    if (input[3] <= -0.034450000151991844) {
                        if (input[3] <= -0.0613000001758337) {
                            memcpy(var95, __extension__ (const double[]){0.45555555555555555, 0.14619883040935672, 0.39824561403508774}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.385, 0.115, 0.5}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.024050000123679638) {
                            memcpy(var95, __extension__ (const double[]){0.7021276595744681, 0.10638297872340426, 0.19148936170212766}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.4774193548387097, 0.12903225806451613, 0.3935483870967742}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.17764999717473984) {
                        if (input[2] <= -0.2644999921321869) {
                            memcpy(var95, __extension__ (const double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.1111111111111111, 0.7777777777777778, 0.1111111111111111}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[3] <= -0.03350000164937228) {
                            memcpy(var95, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= -0.36675000190734863) {
                    memcpy(var95, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= -0.25885000079870224) {
                        if (input[3] <= 0.057499999180436134) {
                            memcpy(var95, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.02044999971985817) {
                            memcpy(var95, __extension__ (const double[]){0.0, 0.25, 0.75}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[1] <= 0.0645500011742115) {
                memcpy(var95, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            } else {
                if (input[1] <= 0.06685000285506248) {
                    memcpy(var95, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 0.06874999776482582) {
                        memcpy(var95, __extension__ (const double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.04124999977648258) {
                            memcpy(var95, __extension__ (const double[]){0.47562189054726367, 0.08358208955223881, 0.4407960199004975}, 3 * sizeof(double));
                        } else {
                            memcpy(var95, __extension__ (const double[]){0.3375, 0.0875, 0.575}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var6, var95, 3, var5);
    double var96[3];
    if (input[2] <= 0.06509999930858612) {
        if (input[0] <= 1.25) {
            memcpy(var96, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        } else {
            if (input[3] <= -0.06764999777078629) {
                if (input[2] <= -0.13325000554323196) {
                    if (input[1] <= 0.07459999993443489) {
                        if (input[2] <= -0.2958499938249588) {
                            memcpy(var96, __extension__ (const double[]){0.5296610169491526, 0.09322033898305085, 0.3771186440677966}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.43672910004568294, 0.15440840566468708, 0.40886249428962995}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.1468999981880188) {
                            memcpy(var96, __extension__ (const double[]){0.40860215053763443, 0.05734767025089606, 0.5340501792114696}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.5, 0.19736842105263158, 0.3026315789473684}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.13295000046491623) {
                        memcpy(var96, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= -0.06825000047683716) {
                            memcpy(var96, __extension__ (const double[]){0.4859887910328263, 0.13530824659727783, 0.3787029623698959}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.9285714285714286, 0.0, 0.07142857142857142}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 0.04450000077486038) {
                    if (input[2] <= -0.044999999925494194) {
                        if (input[1] <= 0.04064999893307686) {
                            memcpy(var96, __extension__ (const double[]){0.23333333333333334, 0.2, 0.5666666666666667}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.03439999930560589) {
                            memcpy(var96, __extension__ (const double[]){0.44150943396226416, 0.12075471698113208, 0.4377358490566038}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.23076923076923078, 0.6153846153846154, 0.15384615384615385}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.05119999870657921) {
                        if (input[3] <= -0.03674999997019768) {
                            memcpy(var96, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.5, 0.2, 0.3}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.06120000034570694) {
                            memcpy(var96, __extension__ (const double[]){0.21875, 0.0, 0.78125}, 3 * sizeof(double));
                        } else {
                            memcpy(var96, __extension__ (const double[]){0.46745562130177515, 0.0650887573964497, 0.46745562130177515}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        memcpy(var96, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    }
    add_vectors(var5, var96, 3, var4);
    double var97[3];
    if (input[1] <= -0.4145500063896179) {
        memcpy(var97, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[3] <= 0.09474999830126762) {
            if (input[1] <= 0.38450001180171967) {
                if (input[0] <= 1.25) {
                    memcpy(var97, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[2] <= -0.3642999976873398) {
                        if (input[3] <= -0.4658999890089035) {
                            memcpy(var97, __extension__ (const double[]){0.6, 0.1, 0.3}, 3 * sizeof(double));
                        } else {
                            memcpy(var97, __extension__ (const double[]){0.8, 0.2, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.059849999845027924) {
                            memcpy(var97, __extension__ (const double[]){0.46842525979216626, 0.14015454303224087, 0.39142019717559284}, 3 * sizeof(double));
                        } else {
                            memcpy(var97, __extension__ (const double[]){0.4352078239608802, 0.09942950285248574, 0.46536267318663405}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var97, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            memcpy(var97, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var4, var97, 3, var3);
    double var98[3];
    if (input[1] <= -0.4145500063896179) {
        memcpy(var98, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.38450001180171967) {
            if (input[1] <= 0.07459999993443489) {
                if (input[2] <= -0.2016500011086464) {
                    if (input[1] <= -0.13199999928474426) {
                        if (input[3] <= -0.28710000216960907) {
                            memcpy(var98, __extension__ (const double[]){0.26811594202898553, 0.2898550724637681, 0.4420289855072464}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){0.4289044289044289, 0.20279720279720279, 0.3682983682983683}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.05650000087916851) {
                            memcpy(var98, __extension__ (const double[]){0.5576923076923077, 0.08653846153846154, 0.3557692307692308}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){0.4742268041237114, 0.2371134020618557, 0.288659793814433}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= 0.07234999909996986) {
                        if (input[3] <= -0.19884999841451645) {
                            memcpy(var98, __extension__ (const double[]){0.1875, 0.25, 0.5625}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){0.4453004622496148, 0.13944530046224962, 0.4152542372881356}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= -2.299999952316284) {
                            memcpy(var98, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[0] <= -0.4049999713897705) {
                    memcpy(var98, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 0.30914999544620514) {
                        if (input[1] <= 0.28164999186992645) {
                            memcpy(var98, __extension__ (const double[]){0.4146341463414634, 0.07692307692307693, 0.5084427767354597}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){0.21518987341772153, 0.12658227848101267, 0.6582278481012658}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.37655000388622284) {
                            memcpy(var98, __extension__ (const double[]){0.47342995169082125, 0.07971014492753623, 0.4468599033816425}, 3 * sizeof(double));
                        } else {
                            memcpy(var98, __extension__ (const double[]){0.7307692307692307, 0.038461538461538464, 0.23076923076923078}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var98, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var3, var98, 3, var2);
    double var99[3];
    if (input[1] <= -0.41440001130104065) {
        memcpy(var99, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[1] <= 0.37880000472068787) {
            if (input[0] <= 1.25) {
                memcpy(var99, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[2] <= -0.24740000069141388) {
                    if (input[1] <= 0.3541499972343445) {
                        if (input[2] <= -0.26545000076293945) {
                            memcpy(var99, __extension__ (const double[]){0.47041420118343197, 0.14201183431952663, 0.3875739644970414}, 3 * sizeof(double));
                        } else {
                            memcpy(var99, __extension__ (const double[]){0.57847533632287, 0.13901345291479822, 0.2825112107623318}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= -0.2710999995470047) {
                            memcpy(var99, __extension__ (const double[]){0.3333333333333333, 0.18518518518518517, 0.48148148148148145}, 3 * sizeof(double));
                        } else {
                            memcpy(var99, __extension__ (const double[]){0.1111111111111111, 0.0, 0.8888888888888888}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -0.24415000528097153) {
                        if (input[1] <= -0.04299999866634607) {
                            memcpy(var99, __extension__ (const double[]){0.08, 0.2, 0.72}, 3 * sizeof(double));
                        } else {
                            memcpy(var99, __extension__ (const double[]){0.09523809523809523, 0.0, 0.9047619047619048}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 0.08664999902248383) {
                            memcpy(var99, __extension__ (const double[]){0.44750158127767237, 0.14452877925363694, 0.4079696394686907}, 3 * sizeof(double));
                        } else {
                            memcpy(var99, __extension__ (const double[]){0.4372801875732708, 0.07737397420867527, 0.48534583821805394}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var99, __extension__ (const double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var2, var99, 3, var1);
    mul_vector_number(var1, 0.02, 3, var0);
    memcpy(output, var0, 3 * sizeof(double));
}
