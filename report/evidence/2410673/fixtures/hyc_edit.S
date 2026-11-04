    .text
    .option push
    .option norvc
    .p2align 2
    .globl hyc_edit_probe
hyc_edit_probe:
    .globl hyc_edit_site
hyc_edit_site:
    ebreak
    .globl hyc_edit_resume
hyc_edit_resume:
    ret
    .option pop
