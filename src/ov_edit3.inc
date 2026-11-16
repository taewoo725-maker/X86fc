; 만든이: KimTaeWoo
; X86FC 편집기 시작 (오버레이 11) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; F3 로 편집기를 열 때 처음 불림 (본체 do_edit → ov_go 11): 파일 형식 확인·매퍼 쪽 확보·파일 읽기 → 편집기(오버레이 1, ed_resume)로 건너감.
;   큰 파일(버퍼보다 큼)은 창 방식: 여기서는 첫 창을 읽도록 오버레이 10 의 명령 19 로 넘김.
; ----------------------------------------------------------------------
                include "edvars.inc"
                include "edeuc.inc"

mp_putp2:       ld      hl,(mp_put)
                jp      (hl)
mp_getp2:       ld      hl,(mp_get)
                jp      (hl)
e2_unmap:       ld      a,(ed_old)
                jr      mp_putp2
c_back:         pop     hl                      ; ov_ret 버리고 편집기로
                xor     a
                ld      b,a
                inc     a
                jp      ov_go
; ======================================================================
; (0.12.43) 편집기 시작 (F3): 파일 열기·읽기 — 편집기(오버레이 1)에서 옮겨 와 그쪽 자리를 비움.  끝나면 편집기로 건너감
; ======================================================================
map_init:       ld      hl,0
                ld      de,0402h
                xor     a
                ld      b,0
                call    0FFCAh
                ld      a,h
                or      l
                jr      z,mi_no
                ld      (mp_all),hl
                ld      de,3
                add     hl,de
                ld      (mp_fre),hl
                ld      de,33
                add     hl,de                   ; 표 + 36 = PUT_P2
                ld      (mp_put),hl
                ld      de,3
                add     hl,de                   ; 표 + 39 = GET_P2
                ld      (mp_get),hl
                or      a
                ret
mi_no:          scf
                ret
mp_alloc:       xor     a                       ; 사용자 쪽, 기본 매퍼 → CF = 실패, A = 쪽 번호
                ld      b,0
                ld      hl,(mp_all)
                jp      (hl)
do_edit:        ld      hl,EV                   ; (0.12.42) 편집기 변수 비움
                ld      de,EV+1
                ld      bc,EVN-1
                ld      (hl),0
                ldir
                call    cur_ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                ld      (en_p),hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      z,de_f
                ld      hl,txt_notfile
                ld      (msg_p),hl
                ret
de_f:           ld      hl,(en_p)
                ld      de,EN_ATTR
                add     hl,de
                ld      a,(hl)
                and     27h
                ld      (ed_attr),a
                ld      hl,1                    ; (0.12.45) 크기(32비트)를 fsz 에: 버퍼에 다 들어가면 그대로, 아니면 큰 파일(창) 방식
                ld      (win_ln),hl
                ld      hl,(en_p)
                ld      de,EN_SIZE
                add     hl,de
                ld      de,fsz
                ld      bc,4
                ldir
                ld      hl,(fsz+2)
                ld      a,h
                or      l
                jr      nz,de_bg
                ld      de,(fsz)
                ld      hl,EDMAX
                or      a
                sbc     hl,de
                jr      c,de_bg
                ld      (ed_sz),de
                jr      de_t
de_bg:          ld      a,1
                ld      (big),a
de_t:           ld      a,(ed_force)            ; (0.12.27) #EDIT 로 열면 묻지 않음
                or      a
                jr      nz,de_g
                call    ed_istext               ; 글 파일 형식이 아니면 한 번 더 물음
                jr      z,de_g
                ld      hl,txt_edq
                call    popup_yn
                ret     nz
de_g:           call    map_init
                jr      c,de_nm
                call    mp_alloc
                jr      c,de_nm
                ld      (ed_seg),a
                call    mp_alloc                ; (0.12.50) 클립보드용 쪽 (못 얻으면 CPBUF 에 760바이트)
                jr      c,de_g2
                ld      (clip_seg),a
de_g2:          jr      de_go
de_nm:          ld      hl,txt_ednomem
                ld      (msg_p),hl
                ret
de_go:
                ld      hl,(en_p)
                call    disp_name
                ld      hl,nmbuf
                ld      de,v_name
                ld      bc,16
                ldir
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                call    mp_getp2
                ld      (ed_old),a
                ld      a,(ed_seg)
                call    mp_putp2
                ld      a,(big)                 ; 큰 파일: 첫 창은 오버레이 10 의 명령 19 가 읽음
                or      a
                jr      z,de_sm
                ld      hl,EDBUF
                ld      (ed_end),hl
                ld      (ed_cur),hl
                ld      (ed_top),hl
                xor     a
                ld      (ed_mod),a
                ld      (il_first),a
                ld      (hk_st),a
                ld      a,19
                ld      (ed_cmd),a
                call    e2_unmap
                pop     hl                      ; ov_ret 버리고 오버레이 14(큰 파일 명령)로
                ld      a,14
                ld      b,0
                jp      ov_go
de_sm:          ld      de,srcpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      z,de_o
                call    explain_err
                jp      ed_out2
de_o:           ld      a,b
                ld      (ed_h),a
                ld      hl,EDBUF
                ld      (ed_end),hl
                ld      hl,(ed_sz)
                ld      a,h
                or      l
                jr      z,de_r
                ex      de,hl
                ld      hl,(ed_sz)
                ld      de,EDBUF
                ld      a,(ed_h)
                ld      b,a
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,de_rr
                or      a
                jr      z,de_rr
                ld      hl,0
de_rr:          ld      de,EDBUF
                add     hl,de
                ld      (ed_end),hl
de_r:           ld      a,(ed_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      hl,EDBUF
                ld      (ed_cur),hl
                ld      (ed_top),hl
                xor     a
                ld      (ed_mod),a
                ld      (il_first),a
                ld      (hk_st),a
                call    ed_u8ask                ; (0.12.19) 7H: UTF-8 이 아니면 확인 (EUC-KR 등은 ? 로 보이고 바이트는 그대로 저장)
                jp      nz,ed_out2
                call    e2_unmap                ; (오버레이 적재가 8000h 를 덮으므로 쪽 2 를 먼저 돌려놓음)
                jp      c_back                  ; 편집기(오버레이 1)로: 쪽 2 를 돌려놓고 건너감 → ed_resume 가 쪽을 끼우고 화면을 그림
; do_edit 안에서 실패했을 때 쓰는 닫기 (쪽 반납)
ed_out2:        call    hk_off
                xor     a
                ld      (hk_st),a
                call    e2_unmap
                ld      a,(ed_seg)
                ld      b,0
                ld      hl,(mp_fre)
                call    jphl2
                ld      a,(clip_seg)
                or      a
                jr      z,oc_n
                ld      b,0
                ld      hl,(mp_fre)
                call    jphl2
oc_n:
                xor     a
                ld      (CSRSW),a
                ret
jphl2:          jp      (hl)
; 7H 이고 본문이 UTF-8 이 아니면 확인 팝업: Z = 계속, NZ = 닫기
ed_u8ask:       ld      a,(s7_h)
                or      a
                ret     z
                call    ed_u8chk
                ret     z
                ld      a,(big)                 ; (0.12.46) EUC-KR 로 보이면 UTF-8 로 바꿔 열지 물음 (큰 파일 제외)
                or      a
                jr      nz,ua_old
                call    eu_det
                jr      z,ua_old
                ld      hl,txt_euq
                call    popup_yn
                jr      nz,ua_old
                call    eu_conv
                jr      nc,ua_ok
                ld      hl,txt_eubig
                ld      de,txt_ppok
                call    popup_ask
ua_old:         ld      hl,txt_edu8
                jp      popup_yn
ua_ok:          ld      a,1
                ld      (ed_euc),a
                xor     a
                ret
; EUC-KR 인가: 높은 바이트가 모두 A1~FE 두 개 짝이고 한글 짝(첫 바이트 B0~C8)이 하나 이상 → NZ = 맞음
eu_det:         ld      hl,EDBUF
                ld      d,0
ed_l:           ld      bc,(ed_end)
                push    hl
                or      a
                sbc     hl,bc
                pop     hl
                jr      nc,ed_ok
                ld      a,(hl)
                inc     hl
                cp      80h
                jr      c,ed_l
                cp      0A1h
                jr      c,ed_bad
                cp      0FFh
                jr      nc,ed_bad
                ld      e,a
                ld      bc,(ed_end)
                push    hl
                or      a
                sbc     hl,bc
                pop     hl
                jr      nc,ed_bad
                ld      a,(hl)
                inc     hl
                cp      0A1h
                jr      c,ed_bad
                cp      0FFh
                jr      nc,ed_bad
                ld      a,e
                cp      0B0h
                jr      c,ed_l
                cp      0C9h
                jr      nc,ed_l
                inc     d
                jr      ed_l
ed_ok:          ld      a,d
                or      a
                ret
ed_bad:         xor     a
                ret
; 글(EUC-KR)을 UTF-8 로 바꿈 (글 쪽 안에서): 글을 표 자리 바로 아래로 옮기고 앞에서부터 바꿔 씀.  한글 완성형만 바뀌고 기호·한자는 '?'.  C = 너무 커서 못 바꿈(9300바이트까지)
eu_conv:        ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (eu_len),hl             ; 글 길이
                ld      de,9301
                or      a
                sbc     hl,de
                ccf
                ret     c                       ; 9301 이상
                call    eu_load
                jr      z,ec_ok
                scf
                ret
ec_ok:          ld      hl,TBLA                 ; 글을 TBLA 아래로 (뒤에서부터 복사)
                ld      de,(eu_len)
                or      a
                sbc     hl,de
                ld      (eu_src),hl             ; 옮긴 글의 처음
                ex      de,hl                   ; DE = 새 처음
                ld      hl,(ed_end)
                dec     hl                      ; 원래 마지막 바이트
                push    de
                ld      de,TBLA-1
                ld      bc,(eu_len)
                ld      a,b
                or      c
                jr      z,ec_s
                lddr
ec_s:           pop     hl                      ; HL = 읽는 자리
                ld      de,EDBUF                ; DE = 쓰는 자리
ec_l:           push    hl
                push    de
                ld      de,TBLA
                or      a
                sbc     hl,de
                pop     de
                pop     hl
                jr      nc,ec_d
                ld      a,(hl)
                inc     hl
                cp      80h
                jr      nc,ec_h
                ld      (de),a
                inc     de
                jr      ec_l
ec_h:           ld      (eu_c),a
                push    de                      ; 쓰는 자리
                push    hl
                ld      de,TBLA
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,ec_q1                ; 짝이 잘림
                ld      b,(hl)
                inc     hl
                ld      (eu_src),hl             ; 짝 다음 자리
                ld      a,(eu_c)
                cp      0B0h
                jr      c,ec_q
                cp      0C9h
                jr      nc,ec_q
                ld      a,b
                cp      0A1h
                jr      c,ec_q
                cp      0FFh
                jr      nc,ec_q
                ld      a,(eu_c)                ; n = (첫 - B0h) x 94 + (둘째 - A1h)
                sub     0B0h
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                push    hl
                add     hl,de
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                pop     de
                or      a
                sbc     hl,de
                ld      a,b
                sub     0A1h
                ld      e,a
                ld      d,0
                add     hl,de
                call    eu_idx
                ld      a,h
                add     a,0ACh
                ld      h,a                     ; HL = 코드 (AC00h + idx)
                pop     de
                call    ec_put3
                ld      hl,(eu_src)
                jp      ec_l
ec_q:           pop     de
                ld      a,'?'
                ld      (de),a
                inc     de
                ld      hl,(eu_src)
                jp      ec_l
ec_q1:          pop     de
                ld      a,'?'
                ld      (de),a
                inc     de
                jp      ec_l
ec_d:           ld      (ed_end),de
                or      a
                ret
; HL = 코드 포인트, DE = 쓸 자리 → UTF-8 세 바이트
ec_put3:        ld      a,h
                rrca
                rrca
                rrca
                rrca
                and     0Fh
                or      0E0h
                ld      (de),a
                inc     de
                ld      a,h
                and     0Fh
                add     a,a
                add     a,a
                ld      h,a
                ld      a,l
                rlca
                rlca
                and     3
                or      h
                or      80h
                ld      (de),a
                inc     de
                ld      a,l
                and     3Fh
                or      80h
                ld      (de),a
                inc     de
                ret
; 본문(EDBUF ~ ed_end)이 올바른 UTF-8 인가 -> Z = 맞음
ed_u8chk:       ld      hl,EDBUF
u8_l:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,u8_n
                xor     a
                ret
u8_n:           ld      a,(hl)
                inc     hl
                cp      80h
                jr      c,u8_l
                cp      0C2h
                jr      c,u8_bad
                ld      b,1
                cp      0E0h
                jr      c,u8_c
                inc     b
                cp      0F0h
                jr      c,u8_c
                cp      0F5h
                jr      nc,u8_bad
                inc     b
u8_c:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,u8_bad
                ld      a,(hl)
                and     0C0h
                cp      80h
                jr      nz,u8_bad
                inc     hl
                djnz    u8_c
                jr      u8_l
u8_bad:         or      1
                ret
txt_euq:        db      "Convert EUC-KR to UTF-8? (Symbols/Hanja will be lost)",0
txt_eubig:      db      "EUC-KR text too large to convert (9300 bytes max)",0
txt_edu8:       db      "Not valid UTF-8 (EUC-KR?). Shown as ?, bytes kept. Edit anyway?",0
; 형식 검사: TXT INI DOC BAT CFG → Z
ed_istext:      ld      hl,(en_p)
                ld      de,EN_EXT
                add     hl,de
                ld      (ed_t1),hl
                ld      de,ed_exts
                ld      b,5
it_l:           push    bc
                push    de
                ld      hl,(ed_t1)
                ld      b,3
it_c:           ld      a,(de)
                cp      (hl)
                jr      nz,it_n
                inc     hl
                inc     de
                djnz    it_c
                pop     de
                pop     bc
                xor     a
                ret
it_n:           pop     de
                inc     de
                inc     de
                inc     de
                pop     bc
                djnz    it_l
                or      1
                ret
ed_exts:        db      "TXTINIDOCBATCFG"
txt_edq:        db      "Not a text type. Edit anyway?",0
txt_ednomem:    db      "No free mapper page for the editor",0

