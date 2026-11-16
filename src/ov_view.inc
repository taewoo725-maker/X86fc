; 만든이: KimTaeWoo
; X86FC 편집기 (오버레이 1) - SPDX-License-Identifier: GPL-3.0-or-later  (F2 보기는 ov_viewer.inc = 오버레이 4)
; (0.12.42) 편집기 상태 변수는 edvars.inc (쪽 3, LVSTK) 로 옮김: 명령(찾기·바꾸기·줄 이동)은 오버레이 10 (ov_edit2.inc)에서 하고 돌아옴
                include "edvars.inc"
                include "edundo.inc"

; ======================================================================
; F3 보기 + 편집 (메모장식 간단 편집기)
;  글은 매퍼 쪽 하나(16 KB)를 페이지 2 창(8000h)에 끼워 넣어 편집 (여유 쪽이 없으면 못 엶). 파일은 최대 16000 바이트.
;  글쇠: 화살표 이동, Ctrl+↑↓ 한 쪽, HOME 줄 처음, SELECT 줄 끝, BS/DEL 지우기, RETURN 줄 바꿈, TAB, 글자 입력,
;        Ctrl+S(F5) 저장, ESC 끝내기(고친 게 있으면 저장 물음)
; ======================================================================
POSIT           equ     00C6h
; ---- 매퍼 보조: 지원 루틴 표를 찾고 쪽을 할당/반납, 페이지 2 전환 ----
mp_free:        ld      b,0                     ; A = 쪽 번호
                ld      hl,(mp_fre)
                jp      (hl)
mp_putp2:       ld      hl,(mp_put)             ; A = 쪽 번호
                jp      (hl)
mp_getp2:       ld      hl,(mp_get)             ; → A = 지금 페이지 2 쪽
                jp      (hl)
; ---- 시작 ----
ed_draw:        call    ed_chkwin
                call    ed_scroll
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
                ld      d,a                     ; D = 글쇠
                sub     1Ch                     ; (0.12.43) 이동 글쇠(화살표·HOME·SELECT): Shift 와 같이 누르면 선택 시작·확장
                cp      4
                jr      c,ek_mv
                ld      a,d
                cp      0Bh
                jr      z,ek_mv
                cp      18h
                jr      nz,ek_nm
                call    ctrl_down               ; Ctrl+X = 자르기 (SELECT 는 줄 끝)
                jr      z,ek_nm
ek_mv:          call    shift_down
                jr      nz,ek_ns
                ld      hl,(sel_a)
                ld      a,h
                or      l
                jr      nz,ek_mx
                ld      hl,(ed_cur)
                ld      (sel_a),hl
                xor     a
                ld      (sel_m),a
                jr      ek_mx
ek_ns:          ld      a,(sel_m)
                or      a
                jr      nz,ek_mx
                ld      hl,0
                ld      (sel_a),hl
ek_mx:          ld      a,d
                jp      ek_nc
ek_nm:          ld      hl,ek_tab               ; 명령 글쇠 표 (글쇠, 명령 번호)
ek_tl:          ld      a,(hl)
                inc     hl
                or      a
                jr      z,ek_tn
                cp      d
                ld      b,(hl)
                inc     hl
                jr      nz,ek_tl
                jr      ek_cmd
ek_tn:          ld      a,d
                cp      08h                     ; 선택이 있으면 BS·DEL 은 선택을 지움
                jr      z,ek_bd
                cp      7Fh
                jr      nz,ek_b2
ek_bd:          ld      hl,(sel_a)
                ld      a,h
                or      l
                ld      a,d
                jr      z,ek_b2
                ld      b,14
                jr      ek_cmd
ek_b2:          cp      02h                     ; Ctrl+B = 선택 시작 / 해제 (이동해도 안 풀림)
                jr      nz,ek_nc
                ld      hl,(sel_a)
                ld      a,h
                or      l
                ld      hl,0
                jr      nz,ek_b3
                ld      hl,(ed_cur)
                ld      a,1
                ld      (sel_m),a
ek_b3:          ld      (sel_a),hl
                jp      ed_draw
ek_tab:         db      1Ah,15, 06h,1, 10h,2, 0Eh,2, 12h,3, 07h,4, 03h,9, 18h,10, 16h,11, 19h,12, 04h,13, 0
ek_cmd:         ld      a,b
                ld      (ed_cmd),a
                xor     a
                ld      (ed_rp),a
ek_go:          call    hk_off                  ; 오버레이 10 으로: 쪽 2 를 원래대로 돌려놓고(오버레이 적재가 8000h 를 덮으므로) 건너감
                xor     a
                ld      (hk_st),a
                ld      a,(ed_old)
                call    mp_putp2
                pop     hl                      ; 쌓여 있는 ov_ret 버림 (ov_go 가 다시 쌓음)
                ld      a,(ed_cmd)              ; 명령 → 칸: 20·21 = EUC-KR 저장(12), 16~19·22 이상 = 큰 파일(14), 그 밖 = 명령(10)
                ld      b,10
                cp      16
                jr      c,ek_g1
                ld      b,14
                cp      20
                jr      c,ek_g1
                cp      22
                jr      nc,ek_g1
                ld      b,12
ek_g1:          ld      a,b
                ld      b,0
                jp      ov_go
; Shift 키가 눌려 있나: Z = 눌림
shift_down:     push    ix
                ld      a,6
                ld      ix,SNSMAT
                call    bios
                pop     ix
                bit     0,a
                ret
ek_nc:
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
; ---- (0.12.42) 오버레이 10 에서 돌아옴 (진입 1): 쪽 다시 끼우고 화면 다시 그림.  ed_rp = 바꾸기 확인 중
ed_resume:      ld      a,(ed_seg)
                call    mp_putp2
                call    ed_geom
                call    clear_screen
                call    ed_foot
                ld      a,(ed_rp)
                or      a
                jp      z,ed_draw
                call    ed_scroll
                call    ed_paint
                ld      hl,txt_edrp
                ld      de,txt_edrpb
                call    popup_ask
                ld      b,5                     ; Y = 이것 바꾸고 다음, A = 전부, N = 건너뜀, Esc = 그만
                cp      'Y'
                jr      z,er_c
                inc     b
                cp      'A'
                jr      z,er_c
                inc     b
                cp      'N'
                jr      z,er_c
                xor     a
                ld      (ed_rp),a
                call    clear_screen
                call    ed_foot
                jp      ed_draw
er_c:           ld      a,b
                ld      (ed_cmd),a
                jp      ek_go
; ---- (0.12.45) 큰 파일: 커서가 창 끝에 다가오면 오버레이 10 이 창을 옮김 (명령 16)
ed_chkwin:      ld      a,(big)
                or      a
                ret     z
                ld      hl,(ed_end)
                ld      de,(ed_cur)
                or      a
                sbc     hl,de
                ld      de,2000
                or      a
                sbc     hl,de
                jr      nc,cw_u
                ld      a,(bw_dn)
                or      a
                jr      nz,cw_g
cw_u:           ld      hl,(ed_cur)
                ld      de,EDBUF+1500
                or      a
                sbc     hl,de
                ret     nc
                ld      a,(bw_up)
                or      a
                ret     z
cw_g:           pop     hl                      ; (부른 자리로 안 돌아감)
                ld      b,16
                jp      ek_cmd
; ---- 닫기 ----
ed_out:         call    hk_off                  ; (0.12.19) 한/영(KANA) 잠금과 램프 끔
                xor     a
                ld      (hk_st),a
                ld      a,(ed_old)
                call    mp_putp2
                ld      a,(ed_seg)
                call    mp_free
                ld      a,(clip_seg)            ; (0.12.50) 클립보드 쪽도 반납
                or      a
                call    nz,mp_free
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
ed_sq:          ld      a,(ed_euc)
                or      a
                ld      b,21
                jp      nz,ek_cmd
                ld      a,(big)
                or      a
                jr      z,sq_s
                ld      b,18
                jp      ek_cmd
sq_s:           ld      b,26                    ; (0.12.49) 작은 파일 저장도 오버레이 14 로 (오버레이 1 을 줄임: 8000h 아래 유지)
                jp      ek_cmd
ed_save:        ld      a,(ed_euc)              ; (0.12.46) EUC-KR 로 열었으면 오버레이 12 가 EUC-KR 로 되돌려 씀
                or      a
                ld      b,20
                jp      nz,ek_cmd
                ld      a,(big)                 ; (0.12.45) 큰 파일은 오버레이 10 이 파일 전체를 다시 씀
                or      a
                jr      z,es_s
                ld      b,17
                jp      ek_cmd
es_s:           ld      b,25
                jp      ek_cmd
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
                ld      hl,0
                ld      (sel_a),hl
                ld      hl,(ed_end)
                ld      de,EDBUF+EDMAX
                or      a
                sbc     hl,de
                ret     nc
                ld      hl,(ed_cur)             ; (0.12.44) 되돌리기 기록: 한 바이트 끼움
                ld      (sp_pos),hl
                ld      hl,0
                ld      (sp_dl),hl
                inc     hl
                ld      (sp_sl),hl
                call    ud_push
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
ed_delc:        ld      hl,0
                ld      (sel_a),hl
                call    ed_lnum
                ld      hl,(ed_cur)
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
ed_del_cur:     ld      hl,0
                ld      (sel_a),hl
                ld      hl,(ed_end)
                ld      de,(ed_cur)
                or      a
                sbc     hl,de
                ret     z
                ret     c
                push    hl
                ld      hl,(ed_cur)             ; (0.12.44) 되돌리기 기록: 한 바이트 지움
                ld      (sp_pos),hl
                ld      hl,1
                ld      (sp_dl),hl
                dec     hl
                ld      (sp_sl),hl
                call    ud_push
                pop     hl
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
ed_paint:       call    ed_selr
                ld      a,(s7_on)
                or      a
                jr      nz,ep_nb
                ld      hl,IMG                  ; SCREEN 0: 깜박임 표 그림을 비움
                ld      de,IMG+1
                ld      bc,239
                ld      (hl),0
                ldir
ep_nb:          ld      hl,hbuf
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
ep_i:           call    ed_lnum                 ; HL = 줄, A = 칸
                push    af
                ld      de,hbuf+62
                call    put_dec16
                ld      a,':'
                ld      (de),a
                inc     de
                pop     af
                ld      l,a
                ld      h,0
                call    put_dec16
                xor     a
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
                call    ed_hlrow
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
; 커서 = 깜박임 표(T2) 그림(IMG)에 그 칸의 비트를 켜고(선택 칸은 줄을 그릴 때 이미 켬) 표를 VRAM 0800h 로 보냄
                ld      a,(ed_row)
                inc     a
                call    row_addr                ; HL = 줄 × 80
                ld      a,(ed_cc)
                ld      e,a
                ld      d,0
                add     hl,de
                call    ed_sbit
                ld      hl,0800h
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,IMG
                ld      b,240
                otir
                ld      b,30
                xor     a
ehl_z:          out     (c),a
                djnz    ehl_z
                ret
; 그림에서 칸 번호 HL 의 비트를 켬
ed_sbit:        ld      a,l
                and     7
                ld      b,a
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                ld      de,IMG
                add     hl,de
                ld      a,80h
                inc     b
                jr      esb_s
esb_r:          srl     a
esb_s:          djnz    esb_r
                or      (hl)
                ld      (hl),a
                ret
; 선택 범위 (작은 쪽 sel_lo, 큰 쪽 sel_hi) 계산.  선택이 없으면 sel_hi = 0
ed_selr:        ld      hl,(sel_a)
                ld      de,(ed_cur)
                ld      a,h
                or      l
                jr      z,sr_n
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,sr_s
                ex      de,hl
sr_s:           ld      (sel_lo),hl
                ld      (sel_hi),de
                ret
sr_n:           ld      (sel_hi),hl
                ret
; 방금 그린 줄(hbuf, v_row)의 선택 칸을 강조: SCREEN 0 = 그림에 비트, 7H·7 = 그 칸을 강조 색으로 다시 찍음
ed_hlrow:       ld      a,(hs_c0)
                ld      c,a
                ld      a,(hs_c1)
                sub     c
                ret     z
                ret     c
                ld      (hs_c1),a               ; = 칸 수
                ld      a,(s7_on)
                or      a
                jr      nz,hl_7
                ld      a,(v_row)
                inc     a
                call    row_addr
                ld      a,(hs_c0)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      a,(hs_c1)
                ld      b,a
hl_l:           push    hl
                push    bc
                call    ed_sbit
                pop     bc
                pop     hl
                inc     hl
                djnz    hl_l
                ret
hl_7:           ld      a,(v_row)
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
hl_h:           ld      (hl),a
                inc     hl
                djnz    hl_h
                ld      a,(hs_c0)
                ld      (s7_col),a
                ld      e,a
                ld      d,0
                ld      hl,hbuf
                add     hl,de
                ld      (s7_tp),hl
                ld      a,(hs_c1)
                ld      (s7_n),a
                ld      a,9
                ld      (s7_pair),a
                call    s7_draw
                xor     a
                ld      (s7_pair),a
                ret
; HL = 줄 시작 → hbuf 에 그 줄 (끝/빈 곳은 공백)
ed_rowbuild:    push    hl
                ld      a,0FFh                  ; (0.12.43) 이 줄의 선택 칸 범위 비움
                ld      (hs_c0),a
                xor     a
                ld      (hs_c1),a
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
                call    rb_sel
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
; 이 글자(HL, 폭 C)가 선택 안에 있으면 그 칸 범위를 hs_c0..hs_c1 에 넓힘.  BC·HL 보존
rb_sel:         ld de,(sel_hi)
                ld a,d
                or e
                ret z
                push hl
                or a
                sbc hl,de
                pop hl
                ret nc
                push hl
                ld de,(sel_lo)
                or a
                sbc hl,de
                pop hl
                ret c
                ld a,(hs_c0)
                cp 0FFh
                jr nz,rs_e
                ld a,(rb_c)
                ld (hs_c0),a
rs_e:           ld a,(rb_c)
                add a,c
                ld (hs_c1),a
                ret
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
; ---- (0.12.42) 커서의 줄 번호(HL, 1부터)와 칸(A, 1부터).  줄 번호는 마지막으로 센 자리(ln_p, ln_n)에서 이어 셈
ed_lnum:        ld      de,(ln_p)
                ld      a,d
                or      e
                jr      nz,ln_k
                ld      de,EDBUF
                ld      (ln_p),de
                ld      hl,(win_ln)
                ld      (ln_n),hl
ln_k:           ld      hl,(ed_cur)
                or      a
                sbc     hl,de
                jr      c,ln_b
                ld      b,h                     ; 앞으로: ln_p ~ cur 의 줄바꿈 수만큼 더함
                ld      c,l
                ld      hl,(ln_p)
                ld      de,(ln_n)
ln_f:           ld      a,b
                or      c
                jr      z,ln_s
                ld      a,0Ah
                cpir
                jr      nz,ln_s
                inc     de
                jp      pe,ln_f
                jr      ln_s
ln_b:           ld      hl,(ln_p)               ; 뒤로: cur ~ ln_p 의 줄바꿈 수만큼 뺌
                ld      bc,(ed_cur)
                or      a
                sbc     hl,bc
                ld      b,h
                ld      c,l
                ld      hl,(ed_cur)
                ld      de,(ln_n)
ln_bl:          ld      a,b
                or      c
                jr      z,ln_s
                ld      a,0Ah
                cpir
                jr      nz,ln_s
                dec     de
                jp      pe,ln_bl
ln_s:           ld      (ln_n),de
                ld      hl,(ed_cur)
                ld      (ln_p),hl
                ld      c,1                     ; 칸 = 줄 처음부터 센 글자 수 + 1 (7H 에서는 UTF-8 이어지는 바이트 제외)
ln_c:           ld      de,EDBUF
                or      a
                sbc     hl,de
                jr      z,ln_e
                add     hl,de
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      z,ln_e
                ld      b,a
                ld      a,(s7_h)
                or      a
                jr      z,ln_i
                ld      a,b
                and     0C0h
                cp      80h
                jr      z,ln_c
ln_i:           inc     c
                jr      nz,ln_c
                dec     c
ln_e:           ld      a,c
                ld      hl,(ln_n)
                ret
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
txt_edhan:      db      "[HAN]",0
txt_edrp:       db      "Replace?",0
txt_edrpb:      db      "[Y] Yes  [N] Skip  [A] All  [Esc] Stop",0
txt_edsave:     db      "Save changes?",0
txt_edsb:       db      "[Y] Yes   [N] No   [Esc] Cancel",0
txt_edname:     db      "EDIT ",0
txt_edmod:      db      "  [modified]",0
txt_edfoot:     db      "^S Save ^F Find F3 Next ^R Repl ^G Line ^B Mark ^C/^X/^V ^Y Del ^D Dup ^Z Undo",0

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
