; 만든이: KimTaeWoo
; X86FC ZIP 풀기 (오버레이 8) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; ov_lzh.inc 와 같은 방식: 오버레이 창이 아니라 매퍼 쪽 하나를 페이지 2(8000h)에 끼워 읽어 와서 실행한다.
; 방식: 저장('S') 와 deflate('Z').  CRC32.  (0.12.40) deflate 의 창: 코드 쪽 안에 가까운 창 8KB(덩어리) + 덩어리가 찰 때마다
;   여분 매퍼 쪽(최대 2쪽, 쪽마다 덩어리 2칸)에 저장.  여분 쪽 2개 = 32KB 이상, 1개 = 16~24KB, 0개 = 8KB.  모자라면 P_RES = 6.
; 매개변수 블록(8003h~)은 ov_lzh.inc 와 같은 자리.  P_RES: 0 성공, 1 CRC 오류, 2 ESC 중단, 3 쓰기 오류, 4 자료 오류, 5 지원 안 함, 6 창이 모자람.
; ----------------------------------------------------------------------
P_METH          equ     8003h                   ; 'S' 저장, 'Z' deflate
P_HIN           equ     8004h
P_HOUT          equ     8005h
P_CSZ           equ     8006h
P_OSZ           equ     800Ah
P_RES           equ     800Eh
P_TXT           equ     800Fh
P_CRC           equ     8011h                   ; 머리의 CRC32 (4)
P_CRCO          equ     8015h                   ; 계산한 CRC32 (4)
INBUF           equ     9E00h                   ; 입력 버퍼 512 (0.12.48: 코드가 늘어 8C00h 에서 옮김)
LITS            equ     8E00h                   ; 글자·길이 부호 표 틀 (34 + 2 x 288)
DSTS            equ     9070h                   ; 거리 부호 표 틀
CLS             equ     90E0h                   ; 길이 부호의 부호 표 틀
CLL             equ     9128h                   ; 그 길이들 (19)
LENS            equ     9140h                   ; 글자·거리 길이들 (320)
CRCT            equ     9600h                   ; CRC32 표 (1024)
WIN             equ     0A000h                  ; 가까운 창 8KB (끝 C000h) = '덩어리' 하나
WENDH           equ     0C0h
WEND            equ     0C000h
WSIZEH          equ     20h
; (0.12.40) 32KB 창: 덩어리(8KB)가 다 차면 매퍼 쪽 최대 2쪽(far)에 돌려 가며 저장. 쪽 2 는 코드 쪽이 끼워져 있으니
;   쪽 전환·복사는 쪽 3 에 복사해 둔 보조 루틴(HLPA)이 한다. 완충(BNC)도 쪽 3.
ZMPALL          equ     LVSTK+96+43             ; 드라이버(ov_arc)의 AV: mp_all, mp_fre, mp_put, ar_seg (ov_arc.inc 와 같아야 함)
ZMPFRE          equ     LVSTK+96+45
ZMPPUT          equ     LVSTK+96+47
ZARSEG          equ     LVSTK+96+29
BNC             equ     LVSTK+272               ; 완충 256
HLPA            equ     LVSTK+528               ; 보조 루틴 자리 (쪽 3)
TB_CONST        equ     34
TB_SYM          equ     37

                org     8000h
                jp      dz_run
                ds      1Dh

dz_run:         call    setup_far
                call    dz_core
                jp      free_far

dz_core:        xor     a
                ld      (P_RES),a
                ld      (err),a
                ld      (flcnt),a
                ld      (bcnt),a
                ld      hl,0
                ld      (bbuf),hl
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
                cp      'S'
                jp      z,dz_stored
                cp      'Z'
                jr      z,dz_def
                ld      a,5
                ld      (P_RES),a
                ret
; ---- deflate 블록 반복
dz_def:         call    getbit                  ; 마지막 블록 표시
                ld      a,0
                rla
                ld      (fin),a
                ld      b,2
                call    gbits
                ld      a,l
                or      a
                jr      z,db_st
                cp      1
                jr      z,db_fx
                cp      2
                jp      nz,dz_bad
                call    rd_dyn                  ; 동적 허프만 표 읽기
                jr      db_sy
db_fx:          call    mk_fixed
                jr      db_sy
db_st:          call    stored_blk              ; 블록 안 저장 (LEN, NLEN)
                jr      db_nx
db_sy:          call    mk_luts
                call    inflate
db_nx:          ld      a,(P_RES)
                or      a
                ret     nz
                ld      a,(err)
                or      a
                jp      nz,dz_bad
                ld      a,(fin)
                or      a
                jr      z,dz_def
                jp      dz_fin

; ---- 저장 방식(ZIP 방식 0): 압축 크기만큼 읽어 그대로 씀
dz_stored:      ld      a,(P_RES)
                or      a
                ret     nz
                call    done_chk
                jr      nc,dz_fin
                call    fetch
                ld      b,a
                ld      a,(err)
                or      a
                jp      nz,dz_bad
                ld      a,b
                call    putb
                jr      dz_stored
; ---- deflate 저장 블록
stored_blk:     ld      a,(bcnt)                ; 바이트 경계로
                and     7
                ld      b,a
                call    gbits
                ld      b,16
                call    gbits                   ; LEN
                ld      (blk),hl
                ld      b,16
                call    gbits                   ; NLEN (검사 생략)
sb_l:           ld      hl,(blk)
                ld      a,h
                or      l
                ret     z
                dec     hl
                ld      (blk),hl
                ld      b,8
                call    gbits
                ld      a,(err)
                or      a
                ret     nz
                ld      a,l
                call    putb
                jr      sb_l
; ---- 끝 처리: CRC32 비교
dz_fin:         call    flush
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
dz_bad:         ld      a,(err)
                cp      6
                ld      a,6
                jr      z,dzb_s
                ld      a,4
dzb_s:          ld      (P_RES),a
                ret

; 원래 크기만큼 다 풀었는가: carry = 아직 더 풀어야 함
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

; ---- 입력 한 바이트 → A (다 떨어지면 0 을 주고 pad 를 올림: 마지막 글자 읽기에서 한 바이트 더 미리 볼 수 있음)
fetch:          ld      hl,(inp)
                ld      de,(ine)
                or      a
                sbc     hl,de
                jr      nz,ft_g
                ld      hl,(inl)
                ld      de,(inl+2)
                ld      a,h
                or      l
                or      d
                or      e
                jr      z,ft_eof
                ld      a,d
                or      e
                jr      nz,ft_1k
                ld      a,h
                cp      2
                jr      c,ft_rd
ft_1k:          ld      hl,512
ft_rd:          ld      a,(P_HIN)
                ld      b,a
                ld      de,INBUF
                ld      c,48h
                call    5
                cp      0C7h
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
                pop     de
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
ft_eof:         ld      a,(pad)
                inc     a
                ld      (pad),a
                cp      4                       ; 4 바이트 넘게 더 읽으면 자료 오류
                jr      c,ft_z
                ld      a,4
                ld      (err),a
ft_z:           xor     a
                ret

; ---- 비트 하나 (낮은 비트부터) → carry.  A·F 만 바꿈
getbit:         push    hl
                ld      a,(bcnt)
                or      a
                jr      nz,gb_h
                push    de
                push    bc
                call    fetch
                pop     bc
                pop     de
                ld      (bbuf),a
                xor     a
                ld      (bbuf+1),a
                ld      a,8
gb_h:           dec     a
                ld      (bcnt),a
                ld      hl,(bbuf)
                srl     h
                rr      l
                ld      (bbuf),hl
                pop     hl
                ret
; B 비트 → HL (낮은 비트부터).  B 를 씀.  (0.12.48) 완충에서 한꺼번에: 8비트 이하는 마스크로, 더 많으면 아랫 8비트 + 윗부분
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
                ld      (gb_t),a
                ld      b,8
                call    gb_s
                push    hl
                ld      a,(gb_t)
                ld      b,a
                call    gb_s
                ld      h,l
                pop     de
                ld      l,e
                ret
gb_s:           ld      a,(bcnt)                ; B (1~8) 비트: 완충이 모자라면 한 바이트 더
                cp      b
                jr      nc,gs_o
                push    bc
                call    fill8
                pop     bc
gs_o:           ld      a,(bcnt)
                sub     b
                ld      (bcnt),a
                ld      a,1
                ld      e,b
gs_p:           add     a,a
                dec     e
                jr      nz,gs_p
                dec     a                       ; A = (1 << B) - 1
                ld      e,a
                ld      hl,(bbuf)
                ld      a,l
                and     e
                ld      e,a                     ; E = 값
gs_r:           srl     h
                rr      l
                djnz    gs_r
                ld      (bbuf),hl
                ld      l,e
                ld      h,0
                ret

; ---- (0.12.48) 첫 8비트 직접 조회표: 8비트 이하 부호는 표 한 번으로, 더 긴 부호는 예전 비트 단위(dsym)로.  낮은 비트부터 오므로
;   표 번호 = 뒤집은 부호.  표 = 글자 낮은 바이트 (LST) + [길이 | 글자 비트8 << 7] (LLT, 0 = 8비트보다 김) 두 쪽
LUTL            equ     9A00h                   ; 글자·길이 부호 조회표 (LST 9A00h, LLT 9B00h)
LUTD            equ     9C00h                   ; 거리 부호 조회표 (LST 9C00h, LLT 9D00h)
; 표 만들기: IX = 틀, A = LST 쪽 번호
mklut:          ld      (ml_pg),a
                ld      h,a
                inc     h
                ld      l,0
                ld      d,h
                ld      e,1
                ld      (hl),0
                ld      bc,255
                ldir
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
                ld      b,a
                ld      hl,(ml_code)
                ld      e,0
ml_r:           srl     h
                rr      l
                rl      e
                djnz    ml_r                    ; E = 뒤집은 부호
                pop     bc
                ld      a,(ml_len)
                ld      d,a
                ld      a,1
ml_sh:          add     a,a
                dec     d
                jr      nz,ml_sh
                ld      d,a                     ; D = 걸음 (길이 8 이면 0 = 한 칸)
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
                ld      a,d
                or      a
                jr      z,ml_nx
                ld      a,e
                add     a,d
                ld      e,a
                jr      nc,ml_f
ml_nx:          ld      hl,(ml_code)
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
mk_luts:        ld      ix,LITS
                ld      a,LUTL>>8
                call    mklut
                ld      ix,DSTS
                ld      a,LUTD>>8
                jp      mklut
; 비트 완충에 8비트 이상 있게 함 (bcnt < 8 일 때 한 바이트 더)
fill8:          call    fetch
                ld      e,a
                ld      d,0
                ld      a,(bcnt)
                ld      b,a
                or      a
                jr      z,f8_s
f8_l:           sla     e
                rl      d
                djnz    f8_l
f8_s:           ld      hl,(bbuf)
                ld      a,l
                or      e
                ld      l,a
                ld      a,h
                or      d
                ld      h,a
                ld      (bbuf),hl
                ld      a,(bcnt)
                add     a,8
                ld      (bcnt),a
                ret
; 글자·길이 부호 하나 → HL        거리 부호 하나 → HL   (IX = 틀: 긴 부호의 비트 단위 읽기용)
dlit:           ld      ix,LITS
                ld      b,LUTL>>8
                jr      lutdec
ddst:           ld      ix,DSTS
                ld      b,LUTD>>8
lutdec:         ld      a,b
                ld      (lt_pg),a
                ld      a,(bcnt)
                cp      8
                call    c,fill8
                ld      a,(lt_pg)
                ld      h,a
                ld      a,(bbuf)
                ld      l,a
                ld      c,(hl)
                inc     h
                ld      a,(hl)
                or      a
                jp      z,dsym                  ; 8비트보다 긴 부호
                ld      b,a
                and     0Fh
                ld      e,a
                ld      a,(bcnt)
                sub     e
                ld      (bcnt),a
                ld      a,b
                rlca
                and     1
                ld      d,a                     ; D = 글자 비트 8
                ld      b,e
                ld      hl,(bbuf)
lt_s:           srl     h
                rr      l
                djnz    lt_s
                ld      (bbuf),hl
                ld      l,c
                ld      h,d
                ret

; ---- 표 만들기: IX = 틀, HL = 길이들, DE = 개수  (ov_lzh.inc 와 같음)
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
                ldir
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
mc_d:           ld      bc,0
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
                ld      hl,(mk_l)
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
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
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
                add     hl,de
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

; ---- 부호 하나 읽기 (정준 부호, 비트 단위, 낮은 비트부터 들어옴): IX = 표 틀 → HL = 글자
dsym:           ld      hl,0
                ld      (d_code),hl
                ld      (d_first),hl
                ld      (d_idx),hl
                push    ix
                pop     hl
                inc     hl
                inc     hl
                ld      (d_cp),hl
                ld      a,15
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
                ex      de,hl
                ld      hl,(d_code)
                or      a
                sbc     hl,de
                jr      nc,ds_m
                ld      hl,(d_code)
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
                ld      a,5                     ; 맞는 부호 없음
                ld      (err),a
                ld      hl,0
                ret

; ---- 고정 허프만 표
mk_fixed:       ld      hl,LENS
                ld      b,144
                ld      a,8
                call    mf_fill
                ld      b,112
                ld      a,9
                call    mf_fill
                ld      b,24
                ld      a,7
                call    mf_fill
                ld      b,8
                ld      a,8
                call    mf_fill
                ld      b,30                    ; 거리 부호 30개, 길이 5
                ld      a,5
                call    mf_fill
                ld      ix,LITS
                ld      hl,LENS
                ld      de,288
                call    mkt
                ld      ix,DSTS
                ld      hl,LENS+288
                ld      de,30
                jp      mkt
mf_fill:        ld      (hl),a
                inc     hl
                djnz    mf_fill
                ret

; ---- 동적 허프만 표 읽기
rd_dyn:         ld      b,5
                call    gbits
                ld      de,257
                add     hl,de
                ld      (nlit),hl
                ld      b,5
                call    gbits
                inc     hl
                ld      (ndst),hl
                ld      b,4
                call    gbits
                ld      de,4
                add     hl,de
                ld      (ncl),hl
                ld      hl,CLL                  ; 길이 부호의 길이들 비움 (19)
                ld      de,CLL+1
                ld      (hl),0
                ld      bc,18
                ldir
                ld      hl,cl_ord
                ld      (rd_p),hl
rd_c1:          ld      hl,(ncl)
                ld      a,h
                or      l
                jr      z,rd_c2
                dec     hl
                ld      (ncl),hl
                ld      b,3
                call    gbits
                ld      a,l
                ld      hl,(rd_p)
                ld      e,(hl)
                inc     hl
                ld      (rd_p),hl
                ld      d,0
                ld      hl,CLL
                add     hl,de
                ld      (hl),a
                jr      rd_c1
rd_c2:          ld      ix,CLS
                ld      hl,CLL
                ld      de,19
                call    mkt
                ld      hl,LENS                 ; 글자·거리 길이들 비움
                ld      de,LENS+1
                ld      (hl),0
                ld      bc,319
                ldir
                ld      hl,(nlit)
                ld      de,(ndst)
                add     hl,de
                ld      (rd_n),hl
                ld      hl,0
                ld      (rd_i),hl
rd_l:           ld      hl,(rd_i)
                ld      de,(rd_n)
                or      a
                sbc     hl,de
                jp      nc,rd_e
                ld      ix,CLS
                call    dsym
                ld      a,(err)
                or      a
                ret     nz
                ld      a,l
                cp      16
                jr      nc,rd_r
                ld      hl,(rd_i)               ; 길이 하나
                ld      de,LENS
                add     hl,de
                ld      (hl),a
                ld      hl,(rd_i)
                inc     hl
                ld      (rd_i),hl
                jr      rd_l
rd_r:           cp      16
                jr      nz,rd_z
                ld      b,2                     ; 16: 앞의 길이를 3~6번 반복
                call    gbits
                ld      de,3
                add     hl,de
                push    hl
                ld      hl,(rd_i)
                ld      de,LENS-1
                add     hl,de
                ld      a,(hl)
                pop     bc
                jr      rd_f
rd_z:           cp      17
                ld      b,3
                ld      de,3
                jr      z,rd_z2
                ld      b,7                     ; 18: 0 을 11~138번
                ld      de,11
rd_z2:          call    gbits
                add     hl,de
                ld      b,h
                ld      c,l
                xor     a
rd_f:           ld      hl,(rd_i)               ; BC 번 A 를 씀
                ld      de,LENS
                add     hl,de
rd_fl:          ld      (hl),a
                inc     hl
                push    hl
                ld      hl,(rd_i)
                inc     hl
                ld      (rd_i),hl
                pop     hl
                dec     bc
                push    af
                ld      a,b
                or      c
                jr      z,rd_fe
                pop     af
                jr      rd_fl
rd_fe:          pop     af
                jr      rd_l
rd_e:           ld      ix,LITS
                ld      hl,LENS
                ld      de,(nlit)
                call    mkt
                ld      ix,DSTS
                ld      hl,(nlit)
                ld      de,LENS
                add     hl,de
                ld      de,(ndst)
                jp      mkt

; ---- 블록 하나 해석 (LITS·DSTS 가 만들어진 상태)
inflate:        call    dlit
                ld      a,(err)
                or      a
                ret     nz
                ld      a,h
                or      a
                jr      nz,in_m
                ld      a,l
                call    putb
                ld      a,(P_RES)
                or      a
                ret     nz
                jr      inflate
in_m:           ld      a,l                     ; 256 = 블록 끝
                or      a
                ret     z
                ld      a,l
                sub     1
                cp      29
                jp      nc,in_bad               ; 257..285 밖
                ld      (lidx),a
                ld      e,a                     ; 길이 = 기본 + 덧비트
                ld      d,0
                ld      hl,lext
                add     hl,de
                ld      b,(hl)
                call    gbits
                push    hl
                ld      a,(lidx)
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      de,lbase
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                add     hl,de
                ld      (mlen),hl
                call    ddst
                ld      a,(err)
                or      a
                ret     nz
                ld      a,l
                cp      30
                jp      nc,in_bad
                ld      (didx),a
                ld      e,a
                ld      d,0
                ld      hl,dext
                add     hl,de
                ld      b,(hl)
                call    gbits
                push    hl
                ld      a,(didx)
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      de,dbase
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                add     hl,de                   ; HL = 거리 (1 부터)
                ex      de,hl
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
                ld      a,(P_RES)
                or      a
                ret     nz
                jp      inflate
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
fm_dn:          jp      inflate
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
                cp      'Z'
                ret     nz
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
                ret
in_bad:         ld      a,5
                ld      (err),a
                ret

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
cl_ord:         db      16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
lbase:          dw      3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258
lext:           db      0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
dbase:          dw      1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577
dext:           db      0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
bbuf:           dw      0
bcnt:           db      0
inp:            dw      0
ine:            dw      0
inl:            dw      0,0
outc:           dw      0,0
crc:            dw      0,0
wptr:           dw      0
flp:            dw      0
fin:            db      0
blk:            dw      0
mlen:           dw      0
lidx:           db      0
didx:           db      0
err:            db      0
pad:            db      0
flcnt:          db      0
ml_pg:          db      0
ml_code:        dw      0
ml_idx:         dw      0
ml_len:         db      0
ml_cnt:         dw      0
lt_pg:          db      0
gb_t:           db      0
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
nlit:           dw      0
ndst:           dw      0
ncl:            dw      0
rd_p:           dw      0
rd_n:           dw      0
rd_i:           dw      0
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
ci_i:           db      0
prgbuf:         ds      24
txtkb:          db      " KB",0
txtstop:        db      "[Esc] Stop",0
