#define    HIPL                1
#define    IP512       2
struct mask {
    char name[80] ; /* name of the mask */
    int  dx ;      /* length in x dimension of the kernel */
    int  dy ;      /* length in y dimension of the kernel */
    char *kernel ;  /* values, row after row. */
};

#define    X(fb)          reg->ipfb_fbreg[fb].ipfb_xu.ipfb_fbx
#define    Y(fb)          reg->ipfb_fbreg[fb].ipfb_yu.ipfb_fby
#define    CSR_LO(fb)     reg->ipfb_fbreg[fb].ipfb_csru.ipfb_fbcsrhl[1]
#define    CSR_HI(fb)     reg->ipfb_fbreg[fb].ipfb_csru.ipfb_fbcsrhl[0]
#define    PAN_LO(fb)     reg->ipfb_fbreg[fb].ipfb_panu.ipfb_fpanhl[1]
#define    PAN_HI(fb)     reg->ipfb_fbreg[fb].ipfb_panu.ipfb_fpanhl[0]
#define    PAN(fb)        reg->ipfb_fbreg[fb].ipfb_panu.ipfb_fbpan
#define    SCROLL_LO(fb)  reg->ipfb_fbreg[fb].ipfb_scrollu.ipfb_fbscrollhl[1]
#define    SCROLL_HI(fb)  reg->ipfb_fbreg[fb].ipfb_scrollu.ipfb_fbscrollhl[0]
#define    SCROLL(fb)     reg->ipfb_fbreg[fb].ipfb_scrollu.ipfb_fbscroll
#define    DATA(fb)       reg->ipfb_fbreg[fb].ipfb_datau.ipfb_fbdata
#define    DATA_HI(fb)    reg->ipfb_fbreg[fb].ipfb_datau.ipfb_fbdatahl[0]
#define    DATA_LO(fb)    reg->ipfb_fbreg[fb].ipfb_datau.ipfb_fbdatahl[1]
#define    MASK_HI(fb)    reg->ipfb_fbreg[fb].ipfb_masku.ipfb_fbmaskhl[0]
#define    MASK_LO(fb)    reg->ipfb_fbreg[fb].ipfb_masku.ipfb_fbmaskhl[1]
#define    WAIT(fb)       while ((CSR_HI(fb) & (IPFB_CSRHCS>>8)) != 0)
#define           WAITVB(fb)     while ((CSR_LO(fb) & IPFB_CSRLVB) == 0)

#define     ALU_K1         reg->ipfb_alureg.ipfb_aluk1
#define     ALU_K2         reg->ipfb_alureg.ipfb_aluk2
#define     ALU_K3         reg->ipfb_alureg.ipfb_aluk3
#define     ALU_IN1        reg->ipfb_alureg.ipfb_input1
#define     ALU_IN2        reg->ipfb_alureg.ipfb_input2
#define     ALU_IN3        reg->ipfb_alureg.ipfb_input3
#define     ALU_OUT        reg->ipfb_alureg.ipfb_output
#define     ALU_SHIFT      reg->ipfb_alureg.ipfb_shift
#define     ALU_CSR        reg->ipfb_alureg.ipfb_alucsr
#define     ALU_MULT       reg->ipfb_alureg.ipfb_mult

