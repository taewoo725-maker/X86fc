; 만든이: KimTaeWoo
; X86FC 편집기 (오버레이 1) - SPDX-License-Identifier: GPL-3.0-or-later  (F2 보기는 ov_viewer.inc = 오버레이 4)
v_name:         ds      16                      ; 파일 이름 (제목 줄)
v_row:          db      0

; ======================================================================
; F3 보기 + 편집 (메모장식 간단 편집기)
;  글은 매퍼 쪽 하나(16 KB)를 페이지 2 창(8000h)에 끼워 넣어 편집 (여유 쪽이 없으면 못 엶). 파일은 최대 16000 바이트.
;  글쇠: 화살표 이동, Ctrl+↑↓ 한 쪽, HOME 줄 처음, SELECT 줄 끝, BS/DEL 지우기, RETURN 줄 바꿈, TAB, 글자 입력,
;        Ctrl+S(F5) 저장, ESC 끝내기(고친 게 있으면 저장 물음)
; ======================================================================
EDMAX           equ     16000
EDBUF           equ     8000h
POSIT           equ     00C6h
; ---- 매퍼 보조: 지원 루틴 표를 찾고 쪽을 할당/반납, 페이지 2 전환 ----
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
mp_free:        ld      b,0                     ; A = 쪽 번호
                ld      hl,(mp_fre)
                jp      (hl)
mp_putp2:       ld      hl,(mp_put)             ; A = 쪽 번호
                jp      (hl)
mp_getp2:       ld      hl,(mp_get)             ; → A = 지금 페이지 2 쪽
                jp      (hl)
; ---- 시작 ----
do_edit:        call    cur_ix
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
                ld      hl,(en_p)               ; 크기: 16000 바이트까지
                ld      de,EN_SIZE
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                inc     hl
                ld      a,(hl)
                inc     hl
                or      (hl)
                jr      nz,de_big
                ld      hl,EDMAX
                or      a
                sbc     hl,de
                jr      c,de_big
                ld      (ed_sz),de
                jr      de_t
de_big:         ld      hl,txt_edbig
                ld      (msg_p),hl
                ret
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
                jr      de_go
de_nm:          ld      hl,txt_ednomem
                ld      (msg_p),hl
                ret
de_go:          call    ed_geom
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
                ld      de,srcpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      z,de_o
                call    explain_err
                jp      ed_out
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
                jp      nz,ed_out
                call    clear_screen
                call    ed_foot
ed_draw:        call    ed_scroll
                call    ed_paint
ed_key:         call    il_wait                 ; (0.12.19) 한/영 키(KANAST)를 보며 기다림. 0 = 한/영 상태가 바뀜
                or      a
                jr      nz,ek_k
                call    hk_sync
                jp      ed_draw
ek_k:           call    ed_han                  ; 한글 모드의 알파벳·조합 중 BS 는 여기서 처리
                jp      c,ed_draw
                cp      1Bh
                jp      z,ed_esc
                cp      13h                     ; Ctrl+S / F5
                jp      z,ed_save
                cp      K_UP
                jp      z,ed_kup
                cp      K_DOWN
                jp      z,ed_kdn
                cp      K_LEFT
                jp      z,ed_klf
                cp      K_RIGHT
                jp      z,ed_krt
                cp      K_HOME
                jp      z,ed_khm
                cp      K_MOVE
                jp      z,ed_ken
                cp      08h
                jp      z,ed_kbs
                cp      7Fh
                jp      z,ed_kdl
                cp      0Dh
                jp      z,ed_kret
                cp      09h
                jr      z,ed_kch
                cp      20h
                jp      c,ed_key
                cp      7Fh
                jp      nc,ed_key
ed_kch:         call    ed_ins1
                jp      ed_draw
; ---- 닫기 ----
ed_out:         call    hk_off                  ; (0.12.19) 한/영(KANA) 잠금과 램프 끔
                xor     a
                ld      (hk_st),a
                ld      a,(ed_old)
                call    mp_putp2
                ld      a,(ed_seg)
                call    mp_free
                xor     a
                ld      (CSRSW),a
                ret
ed_esc:         ld      a,(ed_mod)
                or      a
                jr      z,ed_out
                ld      hl,txt_edsave
                ld      de,txt_edsb
                call    popup_ask
                cp      'Y'
                jr      z,ed_sq
                cp      'N'
                jr      z,ed_out
                call    clear_screen
                call    ed_foot
                jp      ed_draw
ed_sq:          call    ed_wr
                jr      nz,ed_out               ; 저장 실패면 알림만 남고 닫음
                jr      ed_out
ed_save:        call    ed_wr
                jp      ed_draw
; 파일 쓰기: 열어서(읽기·쓰기) 처음부터 쓰고, 짧아졌으면 그 자리에서 잘라 냄
ed_wr:          ld      de,srcpath              ; 같은 이름으로 다시 만듦(내용 비움, 속성 유지)
                ld      a,(ed_attr)
                ld      b,a
                xor     a
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,ew_o
                call    explain_err
                or      1
                ret
ew_o:           ld      a,b
                ld      (ed_h),a
                ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (ed_wn),hl
                ld      a,h
                or      l
                jr      z,ew_t
                ld      de,EDBUF
                ld      a,(ed_h)
                ld      b,a
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      nz,ew_e
ew_t:           xor     a
ew_e:           push    af
                ld      a,(ed_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
                or      a
                jr      z,ew_ok
                call    explain_err
                or      1
                ret
ew_ok:          xor     a
                ld      (ed_mod),a
                ld      hl,txt_edsaved
                ld      (msg_p),hl
                xor     a
                ret
; ---- 글쇠 처리 ----
ed_kup:         call    ctrl_down
                jr      nz,ek_u1
                ld      b,22
ek_ul:          push    bc
                call    ed_up
                pop     bc
                djnz    ek_ul
                jp      ed_draw
ek_u1:          call    ed_up
                jp      ed_draw
ed_kdn:         call    ctrl_down
                jr      nz,ek_d1
                ld      b,22
ek_dl:          push    bc
                call    ed_down
                pop     bc
                djnz    ek_dl
                jp      ed_draw
ek_d1:          call    ed_down
                jp      ed_draw
ed_klf:         ld      hl,(ed_cur)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                jp      z,ed_draw
                ld      hl,(ed_cur)
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,kl_n
                ld      de,EDBUF
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,kl_s
                dec     hl
                ld      a,(hl)
                cp      0Dh
                jr      z,kl_s
                inc     hl
                jr      kl_s
kl_n:           call    ed_lead                 ; (0.12.18) UTF-8 글자 단위: 글자 처음으로
kl_s:           ld      (ed_cur),hl
                jp      ed_draw
ed_krt:         ld      hl,(ed_cur)
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jp      nc,ed_draw
                ld      a,(hl)
                cp      0Dh
                jr      z,kr_c
                call    ed_fwd                  ; (0.12.18) UTF-8 글자 하나 건너뜀
                jr      kr_s
kr_c:           inc     hl
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,kr_s
                ld      a,(hl)
                cp      0Ah
                jr      nz,kr_s
                inc     hl
kr_s:           ld      (ed_cur),hl
                jp      ed_draw
ed_khm:         call    ed_rowstart             ; HL = 줄(화면 줄) 처음
                ld      (ed_cur),hl
                jp      ed_draw
ed_ken:         call    ed_rowstart
                call    ed_nextrow              ; HL = 다음 줄 처음 → 그 앞(줄바꿈 앞)
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,ke_s
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,ke_w
                dec     hl
                ld      a,(hl)
                cp      0Dh
                jr      z,ke_s
                inc     hl
                jr      ke_s
ke_w:           call    ed_lead
ke_s:           ld      (ed_cur),hl
                jp      ed_draw
ed_kbs:         ld      hl,(ed_cur)
                ld      de,EDBUF
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jp      z,ed_draw
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,kb_n
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,kb_d
                dec     hl
                ld      a,(hl)
                cp      0Dh
                jr      z,kb_d
                inc     hl
                jr      kb_d
kb_n:           call    ed_lead
kb_d:           ld      (ed_cur),hl
                call    ed_delc                 ; (0.12.18) 한 글자(UTF-8 3바이트) 지움
                jp      ed_draw
ed_kdl:         call    ed_delc
                jp      ed_draw
ed_kret:        ld      a,0Dh
                call    ed_ins1
                ld      a,0Ah
                call    ed_ins1
                jp      ed_draw
; A = 글자를 커서 앞에 끼워 넣음 (자리가 모자라면 무시)
ed_ins1:        ld      (ed_ch),a
                ld      hl,(ed_end)
                ld      de,EDBUF+EDMAX
                or      a
                sbc     hl,de
                ret     nc
                ld      hl,(ed_end)
                ld      de,(ed_cur)
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l
                ld      a,b
                or      c
                jr      z,ei_p
                ld      hl,(ed_end)
                dec     hl
                ld      de,(ed_end)
                lddr
ei_p:           ld      hl,(ed_cur)
                ld      a,(ed_ch)
                ld      (hl),a
                inc     hl
                ld      (ed_cur),hl
                ld      hl,(ed_end)
                inc     hl
                ld      (ed_end),hl
                ld      a,1
                ld      (ed_mod),a
                ret
; ---- (0.12.19) 한글 입력 (hanin.inc 의 조합기를 재사용: inbuf 에 조합 중인 글자 하나만 넣고 hk_key 를 부른 뒤 결과를 UTF-8 로 본문에 반영) ----
; A = 글쇠 -> C 깃발 = 한글 처리함.  NC 이면 조합을 끝내고 A 는 그대로
ed_han:         ld      c,a
                push    bc
                call    hk_sync
                pop     bc
                ld      a,(s7_h)
                or      a
                jr      z,eh_no
                ld      a,(hk_on)
                or      a
                jr      z,eh_no
                ld      a,c
                cp      08h
                jr      z,eh_bs
                and     0DFh
                sub     'A'
                cp      26
                jr      nc,eh_no
                ld      a,c
                ld      (eh_key),a
                call    eh_prep
                ld      a,(eh_key)
                call    hk_key
                call    eh_apply
                scf
                ret
eh_bs:          ld      a,(hk_st)
                or      a
                jr      z,eh_no
                call    eh_prep
                ld      a,08h
                call    hk_key
                call    eh_apply
                scf
                ret
eh_no:          xor     a
                ld      (hk_st),a
                ld      a,c
                or      a
                ret
; 조합 중인 글자가 있으면 그 쌍을 inbuf 에 넣고 (없으면 비움), eh_was 에 조합 여부 기억
eh_prep:        ld      a,(hk_st)
                ld      (eh_was),a
                or      a
                jr      z,ep0
                call    hk_calc
                ld      hl,inbuf
                ld      (hl),b
                inc     hl
                ld      (hl),c
                inc     hl
                ld      (hl),0
                ld      a,2
                ld      (in_len),a
                ret
ep0:            xor     a
                ld      (inbuf),a
                ld      (in_len),a
                ret
; hk_key 뒤: 조합 중이던 글자(본문의 커서 앞 3바이트)를 지우고 inbuf 의 쌍들을 UTF-8 로 넣음
eh_apply:       ld      a,(eh_was)
                or      a
                jr      z,ea_i
                ld      hl,(ed_cur)
                dec     hl
                dec     hl
                dec     hl
                ld      (ed_cur),hl
                call    ed_del_cur
                call    ed_del_cur
                call    ed_del_cur
ea_i:           ld      a,(in_len)
                srl     a
                ld      (eh_n),a
                ld      hl,inbuf
                ld      (eh_p),hl
ea_l:           ld      a,(eh_n)
                or      a
                ret     z
                dec     a
                ld      (eh_n),a
                ld      hl,(eh_p)
                ld      b,(hl)
                inc     hl
                ld      c,(hl)
                inc     hl
                ld      (eh_p),hl
                ld      a,b                     ; 번호 = (첫째 - 80h) x 128 + (둘째 - 80h), 코드 = AC00h + 번호
                sub     80h
                ld      h,a
                ld      l,0
                srl     h
                rr      l
                ld      a,c
                sub     80h
                or      l
                ld      l,a
                ld      de,0AC00h
                add     hl,de
                ld      (eh_cp),hl
                ld      a,h
                rrca
                rrca
                rrca
                rrca
                and     0Fh
                or      0E0h
                call    ed_ins1
                ld      hl,(eh_cp)
                ld      a,h
                and     0Fh
                add     a,a
                add     a,a
                ld      b,a
                ld      a,l
                rlca
                rlca
                and     3
                or      b
                or      80h
                call    ed_ins1
                ld      a,(eh_cp)
                and     3Fh
                or      80h
                call    ed_ins1
                jr      ea_l
; 7H 이고 본문이 UTF-8 이 아니면 확인 팝업: Z = 계속, NZ = 닫기
ed_u8ask:       ld      a,(s7_h)
                or      a
                ret     z
                call    ed_u8chk
                ret     z
                ld      hl,txt_edu8
                jp      popup_yn
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
txt_edu8:       db      "Not valid UTF-8 (EUC-KR?). Shown as ?, bytes kept. Edit anyway?",0
txt_edhan:      db      "[HAN]",0
eh_was:         db      0
eh_key:         db      0
eh_cp:          dw      0
eh_n:           db      0
eh_p:           dw      0
; ---- (0.12.18) UTF-8 글자 단위 (7H 에서만; 그 밖의 방식은 바이트 단위 그대로) ----
; HL = 글자 처음 -> HL = 다음 글자 처음 (끝 넘지 않음)
ed_fwd:         ld      c,(hl)
                inc     hl
                ld      a,(s7_h)
                or      a
                ret     z
                ld      a,c
                cp      0C0h
                ret     c
                ld      b,1
                cp      0E0h
                jr      c,ef_l
                inc     b
                cp      0F0h
                jr      c,ef_l
                inc     b
ef_l:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                and     0C0h
                cp      80h
                ret     nz
                inc     hl
                djnz    ef_l
                ret
; HL = 글자 안의 한 바이트 -> HL = 그 글자의 처음 (이음 바이트면 앞으로, 최대 3번)
ed_lead:        ld      a,(s7_h)
                or      a
                ret     z
                ld      b,3
el_l:           ld      a,(hl)
                and     0C0h
                cp      80h
                ret     nz
                push    hl
                ld      de,EDBUF
                or      a
                sbc     hl,de
                pop     hl
                ret     z
                dec     hl
                djnz    el_l
                ret
; 커서 자리의 한 글자 지움 (CR LF 는 ed_del1 이, UTF-8 글자는 바이트 수만큼)
ed_delc:        ld      hl,(ed_cur)
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Dh
                jp      z,ed_del1
                call    ed_fwd
                ld      de,(ed_cur)
                or      a
                sbc     hl,de
                ld      b,l
dc_l:           push    bc
                call    ed_del_cur
                pop     bc
                djnz    dc_l
                ret
; 커서 자리의 글자를 지움 (CR LF 짝이면 둘 다)
ed_del1:        ld      hl,(ed_cur)
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Dh
                jr      nz,ed_del_cur
                inc     hl
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,ed_del_cur
                ld      a,(hl)
                cp      0Ah
                jr      nz,ed_del_cur
                call    ed_del_cur
ed_del_cur:     ld      hl,(ed_end)
                ld      de,(ed_cur)
                or      a
                sbc     hl,de
                ret     z
                ret     c
                dec     hl
                ld      b,h
                ld      c,l
                ld      hl,(ed_cur)
                ld      d,h
                ld      e,l
                inc     hl
                ld      a,b
                or      c
                jr      z,edd_n
                ldir
edd_n:           ld      hl,(ed_end)
                dec     hl
                ld      (ed_end),hl
                ld      a,1
                ld      (ed_mod),a
                ret
; ---- 화면 줄 계산 (포인터 방식) ----
; HL = 줄 시작 → HL = 다음 줄 시작
ed_nextrow:     xor     a                       ; 다음 줄 처음 (80칸 폭 기준, 한글 = 2칸, LF 또는 80칸이 차면 줄이 끝남)
                ld      (rb_c),a
nr_l:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Ah
                jr      nz,nr_a
                inc     hl
                ret
nr_a:           cp      0Dh
                jr      nz,nr_c
                inc     hl
                jr      nr_l
nr_c:           call    ed_chr
                ld      a,(rb_c)
                add     a,c
                cp      81
                ret     nc                      ; 이 글자는 다음 줄로 (줄은 그 앞에서 끝남)
                ld      (rb_c),a
                ld      e,b
                ld      d,0
                add     hl,de
                cp      80
                jr      c,nr_l
                ld      de,(ed_end)             ; 80칸이 꽉 참: 바로 뒤의 CR LF 는 이 줄에 붙임
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Dh
                jr      nz,nr_f
                inc     hl
nr_f:           push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Ah
                ret     nz
                inc     hl
                ret
; HL = 줄 시작 → HL = 앞 줄 시작 (맨 앞이면 그대로)
ed_prevrow:     ld      de,EDBUF
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     z
                dec     hl                      ; HL = 바로 앞 글자
                ld      a,(hl)
                cp      0Ah
                jr      nz,pv_c
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,pv_z
                dec     hl
                ld      a,(hl)
                cp      0Dh
                jr      nz,pv_c
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,pv_z
                dec     hl
pv_c:           inc     hl                      ; HL = 마지막 글자 다음 (p+1)
                ld      (pv_e),hl
                ; 이 줄(논리 줄)의 시작 L 찾기: HL 앞 글자가 줄바꿈이거나 맨 앞이면 L = HL
pv_s:           push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,pv_f
                dec     hl
                ld      a,(hl)
                inc     hl
                cp      0Ah
                jr      z,pv_f
                dec     hl
                jr      pv_s
pv_f:           ld      (pv_s0),hl              ; 논리 줄 처음부터 한 화면 줄씩 앞으로 가서 pv_e 를 품은 줄의 처음
                call    ed_nextrow
                ld      de,(pv_e)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,pv_g
                jr      pv_f
pv_g:           ld      hl,(pv_s0)
pv_d:           ret
pv_z:           ld      hl,EDBUF
                ret
; 커서가 있는 화면 줄의 시작 → HL (ed_top 부터 따라감)
ed_rowstart:    ld      hl,(ed_top)
rs_l:           ld      (ed_t2),hl
                call    ed_nextrow
                ld      de,(ed_cur)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,rs_a
                jr      nc,rs_f
rs_a:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                jr      rs_l
rs_f:           ld      hl,(ed_t2)
                ret
; 한 줄 위로: 같은 칸으로
ed_up:          call    ed_rowstart
                ld      (ed_t1),hl              ; 지금 줄 시작
                call    ed_col
                ld      (ed_c1),a
                ld      hl,(ed_t1)
                call    ed_prevrow
                ld      a,(ed_c1)
                jr      ed_goc
ed_down:        call    ed_rowstart
                call    ed_col
                ld      (ed_c1),a
                call    ed_rowstart
                call    ed_nextrow
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc                      ; 마지막 줄
                ld      a,(ed_c1)
ed_goc:         ld      (ed_gt),a               ; HL = 줄 시작, A = 칸 -> 커서 = 그 칸 (줄 끝 넘지 않게, 한글 = 2칸)
                xor     a
                ld      (rb_c),a
gc_l:           ld      a,(ed_gt)
                ld      b,a
                ld      a,(rb_c)
                cp      b
                jr      nc,gc_d
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,gc_d
                ld      a,(hl)
                cp      0Ah
                jr      z,gc_d
                cp      0Dh
                jr      z,gc_d
                call    ed_chr
                ld      a,(rb_c)
                add     a,c
                ld      (rb_c),a
                ld      e,b
                ld      d,0
                add     hl,de
                jr      gc_l
gc_d:           ld      (ed_cur),hl
                ret
; 커서 칸 번호(줄 처음부터 CR 빼고 센 글자 수) → A
ed_col:         call    ed_rowstart             ; 줄 시작 -> 커서까지의 화면 칸 수 (한글 = 2칸) -> A
                ld      de,(ed_cur)
                xor     a
                ld      (rb_c),a
edc_l:          push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,edc_d
                ld      a,(hl)
                cp      0Dh
                jr      nz,edc_c
                inc     hl
                jr      edc_l
edc_c:          push    de
                call    ed_chr
                pop     de
                ld      a,(rb_c)
                add     a,c
                ld      (rb_c),a
                ld      a,l
                add     a,b
                ld      l,a
                jr      nc,edc_l
                inc     h
                jr      edc_l
edc_d:          ld      a,(rb_c)
                ret
; 커서가 화면 안에 오도록 ed_top 조정
ed_scroll:      ld      hl,(ed_cur)             ; 커서가 top 위에 있으면 top 을 줄 단위로 올림
                ld      de,(ed_top)
                or      a
                sbc     hl,de
                jr      nc,es_dn
es_up:          ld      hl,(ed_top)
                call    ed_prevrow
                ld      (ed_top),hl
                ld      de,(ed_cur)
                or      a
                sbc     hl,de                   ; top - cur
                jr      nc,es_up                ; 아직 top >= cur+... 이면 더 올림
                ret
es_dn:          ld      b,0                     ; 커서가 22 줄 밖이면 top 을 내림
es_l:           call    ed_rowno
                ld      hl,ed_rows
                cp      (hl)
                ret     c
                ld      hl,(ed_top)
                call    ed_nextrow
                ld      (ed_top),hl
                jr      es_l
; 커서가 top 에서 몇 번째 줄인가 → A
ed_rowno:       ld      hl,(ed_top)
                ld      c,0
rn_l:           push    bc
                call    ed_nextrow
                pop     bc
                ld      de,(ed_cur)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      z,rn_a
                jr      nc,rn_d
rn_a:           ld      de,(ed_end)
                inc     c
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,rn_d
                jr      rn_l
rn_d:           ld      a,c
                ret
; 화면 그리기 + 커서 위치
ed_paint:       ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      hl,txt_edname
                ld      de,hbuf+1
                call    copy_z
                ld      hl,v_name
                call    copy_z
                ld      a,(ed_mod)
                or      a
                jr      z,ep_h
                ld      hl,txt_edmod
                call    copy_z
ep_h:           ld      a,(hk_on)
                or      a
                jr      z,ep_i
                ld      hl,txt_edhan
                ld      de,hbuf+50
                call    copy_z
ep_i:           xor     a
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      hl,(ed_top)
                xor     a
                ld      (v_row),a
ep_l:           ld      (ed_t1),hl
                call    ed_rowbuild
                ld      a,(v_row)
                inc     a
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      hl,(ed_t1)
                call    ed_nextrow
                ld      a,(v_row)
                inc     a
                ld      (v_row),a
                ld      b,a
                ld      a,(ed_rows)
                cp      b
                jr      z,ep_x
                jr      nc,ep_l
ep_x:
                call    ed_rowno
                ld      (ed_row),a
                call    ed_col
                ld      (ed_cc),a
                ld      a,(s7_on)
                or      a
                jp      nz,s7_edcur
; 커서 = 깜박임 표(T2)로 그 칸만 반전: 표를 0 으로 채우고 (줄+1, 칸) 칸의 비트만 켬
                ld      hl,0800h
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,270
ehl_c:          xor     a
                out     (c),a
                dec     hl
                ld      a,h
                or      l
                jr      nz,ehl_c
                ld      a,(ed_row)
                inc     a
                call    row_addr                ; HL = 줄 × 80
                ld      a,(ed_cc)
                ld      e,a
                ld      d,0
                add     hl,de                   ; HL = 칸 번호
                ld      a,l
                and     7
                ld      b,a
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                ld      de,0800h
                add     hl,de
                ld      a,80h
                inc     b
                jr      ehl_s
ehl_r:          srl     a
ehl_s:          djnz    ehl_r
                push    af
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                pop     af
                out     (c),a
                ret
; HL = 줄 시작 → hbuf 에 그 줄 (끝/빈 곳은 공백)
ed_rowbuild:    push    hl
                ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                pop     hl
                xor     a
                ld      (rb_c),a
rb_l:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                cp      0Ah
                ret     z
                cp      0Dh
                jr      nz,rb_n
                inc     hl
                jr      rb_l
rb_n:           call    ed_chr                  ; B = 바이트 수, C = 폭, DE = 글자 (한글 = 쌍)
                ld      (rb_g),de
                ld      a,b
                ld      (rb_nb),a
                ld      a,c
                or      a
                jr      z,rb_adv
                ld      b,a
                ld      a,(rb_c)
                add     a,b
                cp      81
                ret     nc
                push    hl
                ld      hl,hbuf
                ld      a,(rb_c)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      de,(rb_g)
                ld      (hl),d
                ld      a,b
                cp      2
                jr      nz,rb_1
                inc     hl
                ld      (hl),e
rb_1:           ld      a,(rb_c)
                add     a,b
                ld      (rb_c),a
                pop     hl
rb_adv:         ld      a,(rb_nb)
                ld      e,a
                ld      d,0
                add     hl,de
                jr      rb_l
; HL = 글자 처음 (ed_end 앞) -> B = 바이트 수, C = 화면 폭 (0, 1, 2), D E = 그릴 글자 (폭 2 = 한글 쌍).  HL 보존
;   7H(s7_h)에서만 UTF-8 해석: 한글 음절(EA..ED 3바이트) = 두 칸, 그 밖의 다바이트 = ? 한 칸, 이음 바이트 단독 = 폭 0.  그 밖의 방식은 80h 이상 = '.'
ed_chr:         ld      a,(hl)
                cp      80h
                jr      nc,ec_hi
                cp      09h
                jr      nz,ec_t
                ld      a,' '
ec_t:           cp      20h
                jr      c,ec_dt
                cp      7Fh
                jr      c,ec_ok
ec_dt:          ld      a,'.'
ec_ok:          ld      d,a
                ld      bc,0101h
                ret
ec_hi:          ld      c,a
                ld      a,(s7_h)
                or      a
                jr      z,ec_dt
                ld      a,c
                cp      0C0h
                jr      c,ec_z
                cp      0E0h
                jr      c,ec_q2
                cp      0F0h
                jr      c,ec_h3
                ld      d,'?'
                ld      bc,0401h
                ret
ec_q2:          ld      d,'?'
                ld      bc,0201h
                ret
ec_z:           ld      d,' '
                ld      bc,0100h
                ret
ec_h3:          push    hl
                ld      de,(ed_end)
                inc     hl
                inc     hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,ec_q3
                push    hl
                ld      a,c
                and     0Fh
                rlca
                rlca
                rlca
                rlca
                ld      d,a
                inc     hl
                ld      a,(hl)
                ld      b,a
                and     0C0h
                cp      80h
                jr      nz,ec_q3p
                ld      a,b
                and     3Fh
                ld      b,a
                rrca
                rrca
                and     0Fh
                or      d
                ld      d,a
                ld      a,b
                and     3
                rrca
                rrca
                ld      e,a
                inc     hl
                ld      a,(hl)
                ld      b,a
                and     0C0h
                cp      80h
                jr      nz,ec_q3p
                ld      a,b
                and     3Fh
                or      e
                ld      e,a
                ld      a,d                     ; BOM (U+FEFF) = 폭 0
                cp      0FEh
                jr      nz,ec_nb
                ld      a,e
                cp      0FFh
                jr      nz,ec_nb
                pop     hl
                ld      d,' '
                ld      bc,0300h
                ret
ec_nb:          ld      hl,-0AC00h
                add     hl,de
                jr      nc,ec_q3p
                ld      de,11172
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,ec_q3p
                ld      a,l
                and     7Fh
                or      80h
                ld      e,a
                ld      a,l
                rlca
                ld      a,h
                rla
                or      80h
                ld      d,a
                pop     hl
                ld      bc,0302h
                ret
ec_q3p:         pop     hl
ec_q3:          ld      d,'?'
                ld      bc,0101h
                ret
; 바닥 안내 줄: 7H 는 7H 격자의 맨 아래 줄, 그 밖은 23째 줄 (8점 줄)
ed_foot:        ld      a,(s7_h)
                or      a
                jr      nz,ef_7
                ld      hl,txt_edfoot
                ld      a,23
                jp      qd_line
ef_7:           ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      hl,txt_edfoot
                ld      de,hbuf+2
                call    copy_z
                ld      a,(v_urf)
                call    row_addr
                ld      de,hbuf
                ld      b,80
                jp      scr_write
; 본문 줄 수: 7H = 7H 격자 줄 수 - 2 (제목·안내), 그 밖 22
ed_geom:        ld      a,22
                ld      (ed_rows),a
                ld      a,(s7_h)
                or      a
                ret     z
                ld      a,(v_urf)
                dec     a
                ld      (ed_rows),a
                ret
ed_rows:        db      22
ed_gt:          db      0
rb_c:           db      0
rb_nb:          db      0
rb_g:           dw      0
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
txt_edbig:      db      "File too large to edit (16000 bytes max)",0
txt_ednomem:    db      "No free mapper page for the editor",0
txt_edsave:     db      "Save changes?",0
txt_edsb:       db      "[Y] Yes   [N] No   [Esc] Cancel",0
txt_edsaved:    db      "Saved",0
txt_edname:     db      "EDIT ",0
txt_edmod:      db      "  [modified]",0
txt_edfoot:     db      "Arrows  Ctrl+Up/Dn page  Home/Select line ends  Ctrl+S save  Esc close",0
mp_all:         dw      0
mp_fre:         dw      0
mp_put:         dw      0
mp_get:         dw      0
ed_end:         dw      0
ed_cur:         dw      0
ed_top:         dw      0
ed_sz:          dw      0
ed_wn:          dw      0
ed_t1:          dw      0
pv_e:           dw      0
pv_s0:          dw      0
ed_mod:         db      0
ed_seg:         db      0
ed_old:         db      0
ed_h:           db      0
ed_row:         db      0
ed_c1:          db      0
ed_cc:          db      0
ed_attr:        db      0
ed_ch:          db      0
ed_t2:          dw      0

; ======================================================================
; Ctrl+F 파일 찾기: 활성 패널 폴더에서 아래 하위 폴더까지(깊이 6) 8.3 이름이 틀(* ?)에 맞는 파일을 모아 목록으로 보여 줌.
;  틀에 * ? 가 없으면 이름 속에 그 글이 든 것(*글*)을 찾음. 결과는 28 개까지. ESC 로 찾는 도중에 그만둘 수 있음.
;  목록: 위/아래 고르기, RETURN = 그 폴더로 가서 그 파일에 커서, ESC 닫기.  (긴 이름은 아직 맞추지 않음)
; ======================================================================
; ---- 편집기 커서 (S7): 그 칸만 강조 색으로 다시 찍음
s7_edcur:       ld      hl,(ed_top)
                ld      a,(ed_row)
                or      a
                jr      z,s7ec_g
                ld      b,a
s7ec_n:         push    bc
                call    ed_nextrow
                pop     bc
                djnz    s7ec_n
s7ec_g:         call    ed_rowbuild
                ld      a,(ed_row)
                inc     a
                ld      (s7_row),a
                add     a,a
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_hash
                add     hl,de
                ld      b,4
                ld      a,0FFh
s7ec_i:         ld      (hl),a
                inc     hl
                djnz    s7ec_i
                ld      a,(ed_cc)
                ld      (s7_col),a
                ld      e,a
                ld      d,0
                ld      hl,hbuf
                add     hl,de
                ld      (s7_tp),hl
                ld      b,1
                ld      a,(hl)
                cp      80h
                jr      c,s7ec_1
                inc     hl
                ld      a,(hl)
                cp      80h
                jr      c,s7ec_1
                inc     b
s7ec_1:         ld      a,b
                ld      (s7_n),a
                ld      a,9
                ld      (s7_pair),a
                call    s7_draw
                xor     a
                ld      (s7_pair),a
                ret
