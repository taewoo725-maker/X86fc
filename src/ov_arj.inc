; 만든이: KimTaeWoo
; X86FC ARJ 풀기 (오버레이 9) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; ov_zip.inc 와 같은 방식(코드 쪽 + 창 덩어리 8KB + 여분 쪽 최대 2 = 최대 32KB 이상)으로 쪽 2 에 끼워 실행.
; 방식: 0 저장, 1~3 = LZ77 + 허프만 (ov_lzh.inc 의 -lh5- 와 같은 부호 표, 위치 부호 17개·5비트, 창 26624).  4(빠른 방식)·암호 = 지원 안 함 (P_RES = 5).
; CRC32 (ZIP 과 같음).  P_RES: 0 성공, 1 CRC 오류, 2 ESC 중단, 3 쓰기 오류, 4 자료 오류, 5 지원 안 함, 6 창이 모자람.
; ----------------------------------------------------------------------
P_METH          equ     8003h                   ; 방식 글자 '0' '1' '2' '3'
P_HIN           equ     8004h
P_HOUT          equ     8005h
P_CSZ           equ     8006h
P_OSZ           equ     800Ah
P_RES           equ     800Eh
P_TXT           equ     800Fh
P_CRC           equ     8011h                   ; 머리의 CRC32 (4)
P_CRCO          equ     8015h                   ; 계산한 CRC32 (4)
INBUF           equ     8C00h                   ; 입력 버퍼 512
PTS             equ     8E00h                   ; 길이 부호 표(T) → 위치 부호 표(P) 틀 (80)
PTL             equ     8E60h                   ; 그 길이들 (24)
CTL             equ     8E80h                   ; 글자·길이 부호의 길이들 (512)
CTS             equ     9080h                   ; 글자·길이 부호 표(C) 틀 (1060)
CRCT            equ     9600h                   ; CRC32 표 (1024)
WIN             equ     0A000h                  ; 가까운 창 8KB (덩어리)
WENDH           equ     0C0h
WEND            equ     0C000h
WSIZEH          equ     20h
TB_CONST        equ     34
TB_SYM          equ     37
ZMPALL          equ     LVSTK+96+43             ; 드라이버(ov_arc)의 AV: mp_all, mp_fre, mp_put, ar_seg (ov_arc.inc 와 같아야 함)
ZMPFRE          equ     LVSTK+96+45
ZMPPUT          equ     LVSTK+96+47
ZARSEG          equ     LVSTK+96+29
BNC             equ     LVSTK+272               ; 완충 256
HLPA            equ     LVSTK+528               ; 보조 루틴 자리 (쪽 3)

                org     8000h
                jp      da_run
                ds      1Dh

da_run:         call    setup_far
                call    da_core
                jp      free_far

da_core:        xor     a
                ld      (P_RES),a
                ld      (err),a
                ld      (bn),a
                ld      (pad),a
                ld      (flcnt),a
                ld      hl,0
                ld      (chk),hl
                ld      (outc),hl
                ld      (outc+2),hl
                ld      hl,INBUF
                ld      (inp),hl
                ld      (ine),hl
                ld      hl,(P_CSZ)
                ld      (inl),hl
                ld      hl,(P_CSZ+2)
                ld      (inl+2),hl
                ld      hl,0FFFFh
                ld      (crc),hl
                ld      (crc+2),hl
                call    crc_init
                ld      hl,WIN
                ld      (wptr),hl
                ld      (flp),hl
                ld      a,(P_METH)
                cp      '0'
                jp      z,dc_copy
                cp      '1'
                jr      c,dr_un
                cp      '4'
                jr      c,dr_blk
dr_un:          ld      a,5
                ld      (P_RES),a
                ret
dr_blk:         call    done_chk
                jp      nc,dc_fin
                ld      b,16
                call    gbits                   ; 이 블록의 글자 수
                ld      (blk),hl
                ld      a,h
                or      l
                jp      z,dc_fin
                ld      ix,PTS                  ; 길이 부호 표 (19 글자, 5 비트, 특별 3)
                ld      b,19
                ld      c,5
                ld      d,3
                call    rd_pt
                call    mkP
                call    rd_c
                ld      ix,PTS                  ; 위치 부호 표 (17 글자, 5 비트, 특별 없음)
                ld      b,17
                ld      c,5
                ld      d,0FFh
                call    rd_pt
                call    mkP
dr_sym:         ld      a,(P_RES)
                or      a
                ret     nz
                ld      hl,(blk)
                ld      a,h
                or      l
                jp      z,dr_blk
                dec     hl
                ld      (blk),hl
                call    done_chk
                jp      nc,dc_fin
                call    dsymC                   ; HL = 글자
                ld      a,(err)
                or      a
                jp      nz,dr_bad
                ld      a,h
                or      a
                jr      nz,dr_mt
                ld      a,l                     ; 낱글자
                call    putb
                jr      dr_sym
dr_mt:          ld      de,-253                 ; 길이 = 글자 - 256 + 3
                add     hl,de
                ld      (mlen),hl
                call    dsymP                   ; 위치 부호 j
                ld      a,l
                or      a
                jr      z,dp0                   ; j = 0 → 거리 0
                dec     a
                ld      (xb),a                  ; 추가 비트 수 = j - 1
                ld      b,a
                ld      hl,1
                or      a
                jr      z,dps
dpl:            add     hl,hl                   ; HL = 1 << (j-1)
                djnz    dpl
dps:            push    hl
                ld      a,(xb)
                ld      b,a
                call    gbits
                pop     de
                add     hl,de
                jr      dpd
dp0:            ld      hl,0
dpd:            inc     hl                      ; 뒤로 가는 칸 수 = 거리 + 1
                ex      de,hl                   ; DE = 거리
                ld      a,(err)
                or      a
                jp      nz,dr_bad
                ld      hl,2000h
                or      a
                sbc     hl,de
                jp      c,in_farm               ; 거리 > 8KB: 저장해 둔 덩어리에서
                ld      hl,(wptr)
                or      a
                sbc     hl,de
                ld      a,h
                cp      WIN>>8
                jr      nc,in_ok
                add     a,WSIZEH                ; 창 앞으로 감김: + 창 크기
                ld      h,a
in_ok:          ld      bc,(mlen)               ; HL = 원본
                call    mcopy
                jp      dr_sym
; ---- (0.12.48) 일치 복사를 한꺼번에 (ldir): HL = 원본, BC = 개수 (>= 1).  창 끝(쓰는 쪽·읽는 쪽)마다 끊어 가며
mcopy:          ld      (mc_src),hl
                ld      (mc_cnt),bc
mc_l:           ld      hl,WEND
                ld      de,(wptr)
                or      a
                sbc     hl,de
                ex      de,hl                   ; DE = 창 끝까지 남은 칸 (쓰는 쪽)
                ld      hl,(mc_cnt)
                call    min16
                ld      de,(mc_src)
                push    hl
                ld      hl,WEND
                or      a
                sbc     hl,de
                ex      de,hl                   ; DE = 읽는 쪽 남은 칸
                pop     hl
                call    min16                   ; HL = n
                ld      b,h
                ld      c,l
                ld      (mc_n),hl
                ld      hl,(mc_src)
                ld      de,(wptr)
                ldir
                ld      (mc_src),hl
                ld      (wptr),de
                ld      bc,(mc_n)
                call    oadv
                ld      a,(P_RES)
                or      a
                ret     nz
                ld      hl,(mc_src)
                ld      a,h
                cp      WENDH
                jr      nz,mc_c
                ld      hl,WIN
                ld      (mc_src),hl
mc_c:           ld      hl,(mc_cnt)
                ld      bc,(mc_n)
                or      a
                sbc     hl,bc
                ld      (mc_cnt),hl
                ld      a,h
                or      l
                jr      nz,mc_l
                ret
; BC = 방금 쓴 개수: outc += BC, 창 끝이면 내보내고(flush) 저장(savech) 후 처음으로
oadv:           ld      hl,outc
                ld      a,(hl)
                add     a,c
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,b
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,0
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,0
                ld      (hl),a
                ld      a,(wptr+1)
                cp      WENDH
                ret     nz
                call    flush
                call    savech
                ld      hl,WIN
                ld      (wptr),hl
                ld      (flp),hl
                ret
in_far:         ld      a,6
                ld      (err),a
                jp      dr_bad
in_bad:         ld      a,4
                ld      (err),a
dr_bad:         ld      a,(err)
                cp      6
                ld      a,6
                jr      z,drb_e
                ld      a,4
drb_e:          ld      (P_RES),a
                ret

; ---- 저장 방식: 압축 크기만큼 읽어 그대로 씀
dc_copy:        ld      a,(P_RES)
                or      a
                ret     nz
                call    done_chk
                jr      nc,dc_fin
                call    fetch
                ld      b,a
                ld      a,(err)
                or      a
                jp      nz,dr_bad
                ld      a,b
                call    putb
                jr      dc_copy

; ---- 끝 처리: CRC32 비교
dc_fin:         call    flush
                ld      a,(P_RES)
                or      a
                ret     nz
                ld      hl,crc                  ; 계산값 = ~crc, 머리 값과 비교
                ld      de,P_CRCO
                ld      ix,P_CRC
                ld      b,4
                ld      c,0
dzf_l:          ld      a,(hl)
                cpl
                ld      (de),a
                xor     (ix+0)
                or      c
                ld      c,a
                inc     hl
                inc     de
                inc     ix
                djnz    dzf_l
                ld      a,c
                or      a
                ret     z
                ld      a,1
                ld      (P_RES),a
                ret
done_chk:       ld      hl,(outc)
                ld      de,(P_OSZ)
                or      a
                sbc     hl,de
                ld      hl,(outc+2)
                ld      de,(P_OSZ+2)
                sbc     hl,de
                ret

; 한 글자 A 를 창에 넣고, 창 끝이면 내보내고 처음으로.  출력 개수 +1
putb:           ld      hl,(wptr)
                ld      (hl),a
                inc     hl
                ld      (wptr),hl
                ld      a,h
                cp      WENDH
                jr      nz,pb_c
                call    flush
                call    savech
                ld      hl,WIN
                ld      (wptr),hl
                ld      (flp),hl
pb_c:           ld      hl,outc
                inc     (hl)
                ret     nz
                inc     hl
                inc     (hl)
                ret     nz
                inc     hl
                inc     (hl)
                ret     nz
                inc     hl
                inc     (hl)
                ret

; 아직 안 낸 창 내용(flp ~ wptr)을 파일에 쓰고 CRC32 에 반영.  ESC 확인·진행 표시도 여기서
flush:          ld      hl,(wptr)
                ld      de,(flp)
                or      a
                sbc     hl,de
                jr      z,fl_k
                ld      a,(P_HOUT)
                ld      b,a
                push    de
                push    hl
                ld      c,49h                   ; _WRITE
                call    5
                pop     bc                      ; BC = 바이트 수
                pop     hl                      ; HL = 시작
                or      a
                jr      z,fl_w
                ld      a,3
                ld      (P_RES),a
                ret
fl_w:           call    crcblk
                ld      hl,(wptr)
                ld      (flp),hl
fl_k:           ld      ix,CHSNS                ; ESC 확인
                call    bios
                jr      z,fl_p
                ld      ix,CHGET
                call    bios
                cp      1Bh
                jr      nz,fl_p
                ld      a,2
                ld      (P_RES),a
                ret
fl_p:           ld      a,(flcnt)
                inc     a
                ld      (flcnt),a
                and     3
                ret     nz
; 진행 팝업
prog:           ld      hl,(P_TXT)
                ld      (pp_t1),hl
                ld      de,prgbuf
                ld      hl,outc
                push    de
                call    tokb
                pop     de
                call    put_dec16
                ld      a,' '
                ld      (de),a
                inc     de
                ld      a,'/'
                ld      (de),a
                inc     de
                ld      a,' '
                ld      (de),a
                inc     de
                ld      hl,P_OSZ
                push    de
                call    tokb
                pop     de
                call    put_dec16
                ld      hl,txtkb
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,prgbuf
                ld      (pp_t2),hl
                ld      hl,txtstop
                ld      (pp_t3),hl
                ld      a,34
                ld      (pp_minw),a
                jp      popup_core
tokb:           ld      c,(hl)
                inc     hl
                ld      b,(hl)
                inc     hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ex      de,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                ld      a,b
                srl     a
                srl     a
                or      l
                ld      l,a
                ret

; ---- 입력 한 바이트 → A (다 떨어지거나 읽기 오류면 err)
fetch:          ld      hl,(inp)
                ld      de,(ine)
                or      a
                sbc     hl,de
                jr      nz,ft_g                 ; 버퍼에 남아 있음
                ld      hl,(inl)                ; 남은 압축 크기
                ld      de,(inl+2)
                ld      a,h
                or      l
                or      d
                or      e
                jr      z,ft_eof
                ld      a,d                     ; 읽을 크기 = min(1024, 남은 크기)
                or      e
                jr      nz,ft_1k
                ld      a,h
                cp      2
                jr      c,ft_rd
ft_1k:          ld      hl,512
ft_rd:          ld      a,(P_HIN)
                ld      b,a
                ld      de,INBUF
                ld      c,48h                   ; _READ
                call    5
                cp      0C7h                    ; 끝에 이른 경우도 읽은 만큼은 씀
                jr      z,ft_ok
                or      a
                jr      nz,ft_eof
ft_ok:          ld      a,h
                or      l
                jr      z,ft_eof
                ld      de,INBUF
                ld      (inp),de
                push    hl
                add     hl,de
                ld      (ine),hl
                pop     de                      ; DE = 읽은 수
                ld      hl,(inl)
                or      a
                sbc     hl,de
                ld      (inl),hl
                ld      hl,(inl+2)
                ld      de,0
                sbc     hl,de
                ld      (inl+2),hl
ft_g:           ld      hl,(inp)
                ld      a,(hl)
                inc     hl
                ld      (inp),hl
                ret
ft_eof:         ld      a,(pad)                 ; (0.12.48) 끝에서 미리 읽기: 11바이트까지는 0 으로 봐줌
                inc     a
                ld      (pad),a
                cp      12
                jr      c,ft_z
                ld      a,8
                ld      (err),a
ft_z:           xor     a
                ret

; ---- (0.12.48) 비트 읽기: 완충(bw = 맨 위 비트부터, bn = 들어 있는 비트 수)
; 비트 하나 (맨 위 비트부터) → carry.  A·F 만 바꿈
getbit:         push    hl
                ld      a,(bn)
                or      a
                jr      nz,gb_o
                push    de
                push    bc
                call    fill8
                pop     bc
                pop     de
                ld      a,(bn)
gb_o:           dec     a
                ld      (bn),a
                ld      hl,(bw)
                add     hl,hl
                ld      (bw),hl
                pop     hl
                ret
; 완충에 한 바이트 더 (bn < 8 일 때)
fill8:          call    fetch
                ld      e,a
                ld      d,0
                ld      a,(bn)
                ld      b,a
                ld      a,8
                sub     b
                ld      b,a
f8_l:           sla     e
                rl      d
                djnz    f8_l
                ld      hl,(bw)
                ld      a,l
                or      e
                ld      l,a
                ld      a,h
                or      d
                ld      h,a
                ld      (bw),hl
                ld      a,(bn)
                add     a,8
                ld      (bn),a
                ret
; B 비트 → HL (맨 위 비트부터).  B 를 씀
gbits:          push    de
                push    bc
                call    gb_x
                pop     bc
                pop     de
                ret
gb_x:           ld      a,b
                or      a
                jr      nz,gb_a
                ld      h,a
                ld      l,a
                ret
gb_a:           cp      9
                jr      c,gb_s
                sub     8
                ld      b,a                     ; 위쪽 (B-8) 비트 먼저
                call    gb_s
                push    hl
                ld      b,8
                call    gb_s
                pop     de
                ld      h,e
                ret
gb_s:           ld      a,(bn)                  ; B (1~8) 비트
                cp      b
                jr      nc,gs_o
                push    bc
                call    fill8
                pop     bc
gs_o:           ld      a,(bn)
                sub     b
                ld      (bn),a
                ld      a,8
                sub     b
                ld      c,a
                ld      a,(bw+1)
                inc     c
                dec     c
                jr      z,gs_v
gs_x:           srl     a
                dec     c
                jr      nz,gs_x
gs_v:           ld      e,a
                ld      hl,(bw)
gs_c:           add     hl,hl
                djnz    gs_c
                ld      (bw),hl
                ld      l,e
                ld      h,0
                ret

; ---- 첫 8비트 직접 조회표: 8비트 이하 부호는 표 한 번으로, 더 긴 부호(와 한 글자뿐인 표)는 예전 비트 단위(dsym)로.
;   표 = 글자 낮은 바이트 (LST) + [길이 | 글자 비트8 << 7] (LLT, 0 = 8비트보다 김) 두 쪽.  LUTC = 글자·길이 부호, LUTP = 길이(T)·위치(P) 부호
LUTC            equ     9A00h
LUTP            equ     9C00h
mklut:          ld      (ml_pg),a
                ld      h,a
                inc     h
                ld      l,0
                ld      d,h
                ld      e,1
                ld      (hl),0
                ld      bc,255
                ldir
                ld      a,(ix+TB_CONST)
                or      a
                ret     nz
                ld      hl,0
                ld      (ml_code),hl
                ld      (ml_idx),hl
                ld      a,1
                ld      (ml_len),a
ml_L:           ld      a,(ml_len)
                add     a,a
                ld      e,a
                ld      d,0
                push    ix
                pop     hl
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                ld      (ml_cnt),bc
ml_s:           ld      bc,(ml_cnt)
                ld      a,b
                or      c
                jr      z,ml_n
                dec     bc
                ld      (ml_cnt),bc
                ld      hl,(ml_idx)
                push    hl
                inc     hl
                ld      (ml_idx),hl
                pop     hl
                add     hl,hl
                push    ix
                pop     de
                add     hl,de
                ld      de,TB_SYM
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)                  ; BC = 글자
                push    bc
                ld      a,(ml_len)
                ld      d,a
                ld      a,8
                sub     d
                ld      d,a                     ; D = 8 - 길이
                ld      a,(ml_code)
                ld      b,d
                inc     b
                dec     b
                jr      z,ml_b0
ml_b1:          add     a,a
                djnz    ml_b1
ml_b0:          ld      e,a                     ; E = 시작 번호
                ld      a,1
                ld      b,d
                inc     b
                dec     b
                jr      z,ml_c0
ml_c1:          add     a,a
                djnz    ml_c1
ml_c0:          ld      d,a                     ; D = 채울 개수
                pop     bc
ml_f:           ld      a,(ml_pg)
                ld      h,a
                ld      l,e
                ld      (hl),c
                inc     h
                ld      a,(ml_len)
                bit     0,b
                jr      z,ml_n0
                or      80h
ml_n0:          ld      (hl),a
                inc     e
                dec     d
                jr      nz,ml_f
                ld      hl,(ml_code)
                inc     hl
                ld      (ml_code),hl
                jp      ml_s
ml_n:           ld      hl,(ml_code)
                add     hl,hl
                ld      (ml_code),hl
                ld      a,(ml_len)
                inc     a
                ld      (ml_len),a
                cp      9
                jp      c,ml_L
                ret
mkP:            ld      ix,PTS
                ld      a,LUTP>>8
                jp      mklut
dsymC:          ld      ix,CTS
                ld      b,LUTC>>8
                jr      lutdec
dsymP:          ld      ix,PTS
                ld      b,LUTP>>8
lutdec:         ld      a,(ix+TB_CONST)
                or      a
                jp      nz,dsym
                ld      a,b
                ld      (lt_pg),a
                ld      a,(bn)
                cp      8
                call    c,fill8
                ld      a,(lt_pg)
                ld      h,a
                ld      a,(bw+1)
                ld      l,a
                ld      c,(hl)
                inc     h
                ld      a,(hl)
                or      a
                jp      z,dsym
                ld      b,a
                and     0Fh
                ld      e,a
                ld      a,(bn)
                sub     e
                ld      (bn),a
                ld      a,b
                rlca
                and     1
                ld      d,a
                ld      b,e
                ld      hl,(bw)
lt_s:           add     hl,hl
                djnz    lt_s
                ld      (bw),hl
                ld      l,c
                ld      h,d
                ret

; ---- 표 만들기: IX = 틀, HL = 길이들, DE = 개수
mkt:            ld      (mk_l),hl
                ld      (mk_n),de
                ld      (mk_t),ix
                push    ix
                pop     hl
                ld      d,h
                ld      e,l
                inc     de
                ld      (hl),0
                ld      bc,33
                ldir                            ; 개수[0..16] 비움
                ld      hl,(mk_l)
                ld      bc,(mk_n)
mc:             ld      a,b
                or      c
                jr      z,mc_d
                ld      a,(hl)
                inc     hl
                dec     bc
                or      a
                jr      z,mc
                push    hl
                push    bc
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      de,(mk_t)
                add     hl,de
                inc     (hl)
                jr      nz,mc1
                inc     hl
                inc     (hl)
mc1:            pop     bc
                pop     hl
                jr      mc
mc_d:           ld      bc,0                    ; 길이별 시작 번호 mko[1..16]
                ld      hl,mko
                ld      de,(mk_t)
                inc     de
                inc     de
                ld      a,16
                ld      (mk_i),a
mo_l:           ld      (hl),c
                inc     hl
                ld      (hl),b
                inc     hl
                ld      a,(de)
                inc     de
                add     a,c
                ld      c,a
                ld      a,(de)
                inc     de
                adc     a,b
                ld      b,a
                ld      a,(mk_i)
                dec     a
                ld      (mk_i),a
                jr      nz,mo_l
                ld      hl,(mk_l)               ; 글자들을 시작 번호 순서로 정렬 틀에 넣음
                ld      bc,(mk_n)
                ld      de,0
mf:             ld      a,b
                or      c
                ret     z
                ld      a,(hl)
                inc     hl
                or      a
                jr      z,mf_n
                push    hl
                push    bc
                push    de
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      de,mko-2
                add     hl,de                   ; HL = mko[길이]
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 시작 번호
                push    de
                inc     de
                ld      (hl),d
                dec     hl
                ld      (hl),e
                pop     hl
                add     hl,hl
                ld      de,(mk_t)
                add     hl,de
                ld      de,TB_SYM
                add     hl,de                   ; 정렬 틀 자리
                pop     de
                push    de
                ld      (hl),e
                inc     hl
                ld      (hl),d
                pop     de
                pop     bc
                pop     hl
mf_n:           inc     de
                dec     bc
                jr      mf

; ---- 부호 하나 읽기 (정준 부호, 비트 단위): IX = 표 틀 → HL = 글자
dsym:           ld      a,(ix+TB_CONST)
                or      a
                jr      z,ds_n
                ld      l,(ix+TB_CONST+1)
                ld      h,(ix+TB_CONST+2)
                ret
ds_n:           ld      hl,0
                ld      (d_code),hl
                ld      (d_first),hl
                ld      (d_idx),hl
                push    ix
                pop     hl
                inc     hl
                inc     hl
                ld      (d_cp),hl
                ld      a,16
                ld      (d_len),a
ds_l:           call    getbit
                ld      hl,(d_code)
                adc     hl,hl
                ld      (d_code),hl
                ld      hl,(d_cp)
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                inc     hl
                ld      (d_cp),hl
                ld      hl,(d_first)
                add     hl,bc
                ex      de,hl                   ; DE = first + count
                ld      hl,(d_code)
                or      a
                sbc     hl,de
                jr      nc,ds_m
                ld      hl,(d_code)             ; 찾음: 글자 번호 = idx + (code - first)
                ld      de,(d_first)
                or      a
                sbc     hl,de
                ld      de,(d_idx)
                add     hl,de
                add     hl,hl
                push    ix
                pop     de
                add     hl,de
                ld      de,TB_SYM
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ex      de,hl
                ret
ds_m:           ld      hl,(d_idx)
                add     hl,bc
                ld      (d_idx),hl
                ld      hl,(d_first)
                add     hl,bc
                add     hl,hl
                ld      (d_first),hl
                ld      a,(d_len)
                dec     a
                ld      (d_len),a
                jr      nz,ds_l
                ld      a,7                     ; 어떤 부호와도 안 맞음
                ld      (err),a
                ld      hl,0
                ret

; ---- 길이 부호(T)/위치 부호(P) 표 읽기: IX = 틀, B = 글자 수, C = 비트 수, D = 특별 위치(FFh = 없음)
rd_pt:          ld      a,b
                ld      (rp_n),a
                ld      a,d
                ld      (rp_s),a
                ld      a,c
                ld      (rp_b),a
                ld      b,c
                call    gbits                   ; n
                ld      a,l
                or      a
                jr      nz,rp_g
                ld      a,(rp_b)                ; n = 0: 모든 부호가 한 글자
                ld      b,a
                call    gbits
                ld      (ix+TB_CONST),1
                ld      (ix+TB_CONST+1),l
                ld      (ix+TB_CONST+2),h
                ret
rp_g:           ld      (rp_c),a
                ld      (ix+TB_CONST),0
                ld      hl,PTL                  ; 길이들 비움
                ld      de,PTL+1
                ld      (hl),0
                ld      bc,23
                ldir
                ld      hl,PTL
                ld      (rp_p),hl
                xor     a
                ld      (rp_i),a
rp_l:           ld      a,(rp_i)
                ld      b,a
                ld      a,(rp_c)
                cp      b
                jr      z,rp_e
                jr      c,rp_e
                ld      b,3
                call    gbits
                ld      d,l
                ld      a,d
                cp      7
                jr      nz,rp_v
rp_x:           call    getbit                  ; 7 이상: 1 이 이어지는 만큼 길이 + 1
                jr      nc,rp_v
                inc     d
                jr      rp_x
rp_v:           ld      hl,(rp_p)
                ld      (hl),d
                inc     hl
                ld      (rp_p),hl
                ld      a,(rp_i)
                inc     a
                ld      (rp_i),a
                ld      b,a
                ld      a,(rp_s)
                cp      b
                jr      nz,rp_l
                ld      b,2                     ; 특별 위치 뒤에는 0 길이가 (2 비트 값) 개
                call    gbits
                ld      a,(rp_i)
                add     a,l
                ld      (rp_i),a
                ld      e,l
                ld      d,0
                ld      hl,(rp_p)
                add     hl,de
                ld      (rp_p),hl
                jr      rp_l
rp_e:           ld      hl,PTL
                ld      a,(rp_n)
                ld      e,a
                ld      d,0
                jp      mkt

; ---- 글자·길이 부호(C) 표 읽기
rd_c:           ld      b,9
                call    gbits                   ; n
                ld      a,h
                or      l
                jr      nz,rc_g
                ld      b,9
                call    gbits
                ld      ix,CTS
                ld      (ix+TB_CONST),1
                ld      (ix+TB_CONST+1),l
                ld      (ix+TB_CONST+2),h
                ret
rc_g:           ld      (rc_n),hl
                ld      ix,CTS
                ld      (ix+TB_CONST),0
                ld      hl,CTL                  ; 길이들 비움 (512)
                ld      de,CTL+1
                ld      (hl),0
                ld      bc,511
                ldir
                ld      hl,0
                ld      (rc_i),hl
rc_l:           ld      hl,(rc_i)
                ld      de,(rc_n)
                or      a
                sbc     hl,de
                jr      nc,rc_e
                call    dsymP                   ; T 부호
                ld      a,l
                cp      3
                jr      nc,rc_v
                or      a                       ; 0, 1, 2 = 0 길이 반복 (1, 4비트+3, 9비트+20)
                ld      hl,1
                jr      z,rc_r
                cp      1
                jr      nz,rc_c2
                ld      b,4
                call    gbits
                ld      de,3
                add     hl,de
                jr      rc_r
rc_c2:          ld      b,9
                call    gbits
                ld      de,20
                add     hl,de
rc_r:           ld      de,(rc_i)               ; i += 반복
                add     hl,de
                ld      (rc_i),hl
                jr      rc_l
rc_v:           sub     2
                ld      hl,(rc_i)
                ld      de,CTL
                add     hl,de
                ld      (hl),a
                ld      hl,(rc_i)
                inc     hl
                ld      (rc_i),hl
                jr      rc_l
rc_e:           ld      ix,CTS
                ld      hl,CTL
                ld      de,510
                call    mkt
                ld      ix,CTS
                ld      a,LUTC>>8
                jp      mklut

; ---- (0.12.40) 거리 > 8KB 인 일치: 원본이 이미 저장된 덩어리(far) 안에 있다.  DE = 거리
;   절대 위치 = (덩어리 수 chk) x 8192 + o - 거리,  o = wptr - WIN.   delta = 거리 - o (>= 1)
;   m = 몇 덩어리 전인가 = ceil(delta / 8192),  덩어리 안 위치 = (8192 - delta mod 8192) mod 8192
;   조각 하나씩(<= 256, 원본 덩어리 끝·출력 덩어리 끝을 안 넘게) 완충으로 가져와 putb.  매번 처음부터 다시 계산
in_farm:        ld      (fdist),de
                ld      a,(nfar)
                or      a
                jp      z,in_far                ; 여분 쪽 없음 = 'Need more mapper pages'
fm_lp:          ld      hl,(mlen)
                ld      a,h
                or      l
                jp      z,fm_dn
                ld      hl,(wptr)
                ld      de,WIN
                or      a
                sbc     hl,de                   ; HL = o
                ld      (fo),hl
                ex      de,hl
                ld      hl,(fdist)
                or      a
                sbc     hl,de                   ; HL = delta
                ld      (fdl),hl
                ld      de,1FFFh
                add     hl,de
                ld      a,h
                rlca
                rlca
                rlca
                and     7
                ld      b,a                     ; B = m
                ld      a,(fS)
                cp      b
                jp      c,in_far                ; 저장해 둔 덩어리보다 먼 곳
                ld      hl,(chk)
                ld      a,h
                or      a
                jr      nz,fm_c
                ld      a,l
                cp      b
                jp      c,in_bad                ; 글 시작보다 앞 = 자료 오류
fm_c:           ld      a,(chk)
                sub     b
                ld      c,a
                ld      a,(fmask)
                and     c                       ; A = 칸 번호 (0..S-1)
                ld      c,a
                srl     a                       ; 쪽 번호
                ld      e,a
                ld      d,0
                ld      hl,fseg0
                add     hl,de
                ld      a,(hl)
                ld      (fsg),a
                ld      hl,8000h                ; 쪽 안 주소 = 8000h + (칸 & 1) x 2000h + 덩어리 안 위치
                bit     0,c
                jr      z,fm_a
                ld      hl,0A000h
fm_a:           ld      de,(fdl)
                ld      a,d
                and     1Fh
                ld      d,a
                or      e
                jr      z,fm_s                  ; 위치 0
                push    hl
                ld      hl,2000h
                or      a
                sbc     hl,de                   ; HL = 덩어리 안 위치
                ld      (fsoff),hl
                pop     de
                add     hl,de
                ld      (fsa),hl
                jr      fm_n
fm_s:           ld      (fsa),hl
                ld      hl,0
                ld      (fsoff),hl
fm_n:           ld      hl,256                  ; n = min(256, mlen, 8192 - 위치, 8192 - o)
                ld      de,(mlen)
                call    min16
                ld      de,(fsoff)
                push    hl
                ld      hl,2000h
                or      a
                sbc     hl,de
                ex      de,hl
                pop     hl
                call    min16
                ld      de,(fo)
                push    hl
                ld      hl,2000h
                or      a
                sbc     hl,de
                ex      de,hl
                pop     hl
                call    min16
                ld      (fn),hl
                ld      b,h
                ld      c,l
                ld      hl,(fsa)
                ld      a,(fsg)
                call    HLPA+(hlp_rd-hlp_tpl)   ; 완충(BNC) <- far
                ld      hl,(mlen)               ; mlen -= n
                ld      de,(fn)
                or      a
                sbc     hl,de
                ld      (mlen),hl
                ld      hl,BNC
                ld      bc,(fn)
fm_cp:          ld      de,(wptr)
                ldir
                ld      (wptr),de
                ld      bc,(fn)
                call    oadv
                ld      a,(P_RES)
                or      a
                ret     nz
                jp      fm_lp
fm_dn:          jp      dr_sym
min16:          push    hl                      ; HL = min(HL, DE)
                or      a
                sbc     hl,de
                pop     hl
                ret     c
                ex      de,hl
                ret

; ---- 덩어리(창 8KB)가 다 찼다: far 칸에 저장 (여분 쪽이 있을 때).  완충 256 씩 옮김
savech:         ld      a,(nfar)
                or      a
                jr      z,sv_e
                ld      a,(P_RES)
                or      a
                ret     nz
                ld      a,(chk)
                ld      b,a
                ld      a,(fmask)
                and     b
                ld      c,a
                srl     a
                ld      e,a
                ld      d,0
                ld      hl,fseg0
                add     hl,de
                ld      a,(hl)
                ld      (fsg),a
                ld      hl,8000h
                bit     0,c
                jr      z,sv_a
                ld      hl,0A000h
sv_a:           ld      (fsa),hl
                ld      hl,WIN
                ld      (fsrc),hl
                ld      b,32
sv_l:           push    bc
                ld      hl,(fsrc)
                ld      de,BNC
                ld      bc,256
                ldir
                ld      (fsrc),hl
                ld      a,(fsg)
                ld      de,(fsa)
                ld      bc,256
                call    HLPA+(hlp_wr-hlp_tpl)
                ld      hl,fsa+1
                inc     (hl)
                pop     bc
                djnz    sv_l
sv_e:           ld      hl,(chk)
                inc     hl
                ld      (chk),hl
                ret

; ---- 여분 쪽 확보 (deflate 일 때만, 최대 2) + 보조 루틴을 쪽 3 에 복사
setup_far:      xor     a
                ld      (nfar),a
                ld      (fS),a
                ld      a,(P_METH)
                cp      '1'
                ret     c
                cp      '4'
                ret     nc
                ld      hl,hlp_tpl
                ld      de,HLPA
                ld      bc,hlp_end-hlp_tpl
                ldir
                ld      a,(ZARSEG)
                ld      (HLPA+3),a
                ld      hl,(ZMPPUT)
                ld      (HLPA+1),hl
                ld      hl,fseg0
                ld      b,2
sf_l:           push    hl
                push    bc
                xor     a
                ld      b,a
                ld      hl,(ZMPALL)
                call    jphl
                pop     bc
                pop     hl
                jr      c,sf_e
                ld      (hl),a
                inc     hl
                ld      a,(nfar)
                inc     a
                ld      (nfar),a
                djnz    sf_l
sf_e:           ld      a,(nfar)
                add     a,a
                ld      (fS),a
                dec     a
                ld      (fmask),a
                ret
jphl:           jp      (hl)
free_far:       ld      a,(nfar)
                or      a
                ret     z
                ld      b,a
                ld      hl,fseg0
ff_l:           push    hl
                push    bc
                ld      a,(hl)
                ld      b,0
                ld      hl,(ZMPFRE)
                call    jphl
                pop     bc
                pop     hl
                inc     hl
                djnz    ff_l
                xor     a
                ld      (nfar),a
                ret

; ---- 쪽 3 에 복사되는 보조 루틴 (자리 무관: jr 만 씀).  HLPA+0 = jp (hl), +1 = PUT_P2 주소, +3 = 코드 쪽 번호
;   hlp_rd: A = 쪽 번호, HL = 쪽 2 주소, BC = 수 → BNC 로.   hlp_wr: A = 쪽 번호, DE = 쪽 2 주소, BC = 수 ← BNC.  끝나면 코드 쪽 복귀
hlp_tpl:        db      0E9h
                dw      0
                db      0
hlp_rd:         push    hl
                push    bc
                ld      hl,(HLPA+1)
                call    HLPA
                pop     bc
                pop     hl
                ld      de,BNC
                ldir
                jr      hlp_bk
hlp_wr:         push    de
                push    bc
                ld      hl,(HLPA+1)
                call    HLPA
                pop     bc
                pop     de
                ld      hl,BNC
                ldir
hlp_bk:         ld      a,(HLPA+3)
                ld      hl,(HLPA+1)
                jp      HLPA
hlp_end:
; ---- (0.12.48) CRC32: HL = 시작, BC = 바이트 수.  crc 를 레지스터에 두고 표 하나(바이트마다 한 쪽)로 4바이트씩 풂
crcblk:         push    hl
                pop     ix
                ld      a,c
                and     3
                ld      (cb_r),a
                srl     b
                rr      c
                srl     b
                rr      c
                ld      (cb_b),bc
                ld      a,(crc)
                ld      e,a
                ld      a,(crc+1)
                ld      d,a
                ld      a,(crc+2)
                ld      c,a
                ld      a,(crc+3)
                ld      b,a
cb_l:           ld      hl,(cb_b)
                ld      a,h
                or      l
                jp      z,cb_t
                dec     hl
                ld      (cb_b),hl
                ld      a,(ix+0)
                xor     e
                ld      l,a
                ld      h,CRCT>>8
                ld      a,(hl)
                xor     d
                ld      e,a
                inc     h
                ld      a,(hl)
                xor     c
                ld      d,a
                inc     h
                ld      a,(hl)
                xor     b
                ld      c,a
                inc     h
                ld      b,(hl)
                ld      a,(ix+1)
                xor     e
                ld      l,a
                ld      h,CRCT>>8
                ld      a,(hl)
                xor     d
                ld      e,a
                inc     h
                ld      a,(hl)
                xor     c
                ld      d,a
                inc     h
                ld      a,(hl)
                xor     b
                ld      c,a
                inc     h
                ld      b,(hl)
                ld      a,(ix+2)
                xor     e
                ld      l,a
                ld      h,CRCT>>8
                ld      a,(hl)
                xor     d
                ld      e,a
                inc     h
                ld      a,(hl)
                xor     c
                ld      d,a
                inc     h
                ld      a,(hl)
                xor     b
                ld      c,a
                inc     h
                ld      b,(hl)
                ld      a,(ix+3)
                xor     e
                ld      l,a
                ld      h,CRCT>>8
                ld      a,(hl)
                xor     d
                ld      e,a
                inc     h
                ld      a,(hl)
                xor     c
                ld      d,a
                inc     h
                ld      a,(hl)
                xor     b
                ld      c,a
                inc     h
                ld      b,(hl)
                push    de
                ld      de,4
                add     ix,de
                pop     de
                jp      cb_l
cb_t:           ld      a,(cb_r)
                or      a
                jr      z,cb_e
                ld      (cb_b),a
cb_t1:
                ld      a,(ix+0)
                xor     e
                ld      l,a
                ld      h,CRCT>>8
                ld      a,(hl)
                xor     d
                ld      e,a
                inc     h
                ld      a,(hl)
                xor     c
                ld      d,a
                inc     h
                ld      a,(hl)
                xor     b
                ld      c,a
                inc     h
                ld      b,(hl)
                inc     ix
                ld      hl,cb_b
                dec     (hl)
                jr      nz,cb_t1
cb_e:           ld      a,e
                ld      (crc),a
                ld      a,d
                ld      (crc+1),a
                ld      a,c
                ld      (crc+2),a
                ld      a,b
                ld      (crc+3),a
                ret
; ---- CRC32 표
crc_init:       xor     a
                ld      (ci_i),a
ci_l:           ld      a,(ci_i)
                ld      l,a
                ld      h,0
                ld      de,0
                ld      b,8
ci_b:           srl     d
                rr      e
                rr      h
                rr      l
                jr      nc,ci_n
                ld      a,l
                xor     20h
                ld      l,a
                ld      a,h
                xor     83h
                ld      h,a
                ld      a,e
                xor     0B8h
                ld      e,a
                ld      a,d
                xor     0EDh
                ld      d,a
ci_n:           djnz    ci_b
                ld      a,(ci_i)                ; (0.12.48) 바이트마다 한 쪽: CRCT + 쪽 x 256 + 번호
                ld      c,a
                ld      b,CRCT>>8
                ld      a,l
                ld      (bc),a
                inc     b
                ld      a,h
                ld      (bc),a
                inc     b
                ld      a,e
                ld      (bc),a
                inc     b
                ld      a,d
                ld      (bc),a
                ld      a,(ci_i)
                inc     a
                ld      (ci_i),a
                jr      nz,ci_l
                ret

; ---- 자료
inp:            dw      0
ine:            dw      0
inl:            dw      0,0
outc:           dw      0,0
crc:            dw      0,0
wptr:           dw      0
flp:            dw      0
blk:            dw      0
mlen:           dw      0
xb:             db      0
err:            db      0
flcnt:          db      0
bw:             dw      0
bn:             db      0
pad:            db      0
ml_pg:          db      0
ml_code:        dw      0
ml_idx:         dw      0
ml_len:         db      0
ml_cnt:         dw      0
lt_pg:          db      0
cb_b:           dw      0
cb_r:           db      0
mc_src:         dw      0
mc_cnt:         dw      0
mc_n:           dw      0
chk:            dw      0
nfar:           db      0
fS:             db      0
fmask:          db      0
fseg0:          db      0
fseg1:          db      0
fsg:            db      0
fsa:            dw      0
fsoff:          dw      0
fsrc:           dw      0
fdist:          dw      0
fdl:            dw      0
fo:             dw      0
fn:             dw      0
d_code:         dw      0
d_first:        dw      0
d_idx:          dw      0
d_cp:           dw      0
d_len:          db      0
mk_l:           dw      0
mk_n:           dw      0
mk_t:           dw      0
mk_i:           db      0
mko:            ds      34
rp_n:           db      0
rp_s:           db      0
rp_b:           db      0
rp_c:           db      0
rp_i:           db      0
rp_p:           dw      0
rc_n:           dw      0
rc_i:           dw      0
ci_i:           db      0
prgbuf:         ds      24
txtkb:          db      " KB",0
txtstop:        db      "[Esc] Stop",0
