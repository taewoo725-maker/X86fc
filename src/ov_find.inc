FDMAX           equ     28
FDREC           equ     72
FDDEP           equ     6
do_find:        xor     a
                ld      (fd_mode),a
                ld      (inbuf),a
                ld      (in_len),a
                ld      (il_first),a
                ld      (il_raw),a
                ld      hl,txt_fdq
                call    input_line
                ret     nz
                call    trim_in
                ld      a,(in_len)
                or      a
                ret     z
                ld      hl,inbuf                ; 틀: 대문자로, * ? 가 없으면 *글*
                ld      de,fd_pat+1
                ld      b,20
                ld      c,0
fq_c:           ld      a,(hl)
                or      a
                jr      z,fq_e
                cp      '*'
                jr      z,fq_w
                cp      '?'
                jr      nz,fq_u
fq_w:           inc     c
                jr      fq_cp
fq_u:           cp      'a'
                jr      c,fq_cp
                cp      'z'+1
                jr      nc,fq_cp
                and     0DFh
fq_cp:          ld      (de),a
                inc     hl
                inc     de
                djnz    fq_c
fq_e:           xor     a
                ld      (de),a
                ld      hl,fd_pat+1
                ld      a,c
                or      a
                jr      nz,fq_nw
                ld      hl,fd_pat+1             ; 와일드카드가 없으면 앞뒤에 *
                ld      de,fd_pat+1
                ld      b,0
                call    strlen
                ld      c,a
                ld      b,0
                ld      a,'*'
                ld      (fd_pat),a
                ld      hl,fd_pat+1
                add     hl,bc
                ld      (hl),'*'
                inc     hl
                ld      (hl),0
                ld      hl,fd_pat
                jr      fq_go
fq_nw:          ld      hl,fd_pat+1
fq_go:          ld      (fd_pp),hl
                ; --- 시작 폴더 = 활성 패널 폴더
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,fd_path
                call    copy_z
                xor     a
                ld      (de),a
                ld      (fd_dep),a
                ld      (fd_stop),a
                ld      (ff_n),a
                call    fd_show
fd_lvl:         ld      hl,fd_path              ; 틀 만들기: 경로 + (\ 없으면 \) + *.*
                ld      de,specbuf
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,ffl_s
                ld      a,'\'
                ld      (de),a
                inc     de
ffl_s:           ld      hl,txt_star
                ld      bc,4
                ldir
                ld      de,specbuf
                ld      b,SRCHATTR
                ld      c,F_FFIRST
                call    bdos_fib
                jr      nz,fd_up
fd_ent:         call    fd_chk
                ld      a,(fd_stop)
                or      a
                jp      nz,fd_done
                ld      a,(FIB+14)
                and     08h
                jr      nz,fd_nx
                ld      a,(FIB+1)
                cp      '.'
                jr      z,fd_nx
                ld      a,(FIB+14)
                and     10h
                jr      nz,fd_dir
                ld      a,(fd_mode)
                or      a
                jr      nz,fd_nx
                ld      hl,(fd_pp)              ; 파일: 틀에 맞는가
                ld      de,FIB+1
                call    fd_match
                jr      nz,fd_nx
                call    fd_addres
fd_nx:          ld      c,F_FNEXT
                call    bdos_fib
                jr      z,fd_ent
fd_up:          ld      a,(fd_dep)              ; 이 폴더 끝: 위 폴더로
                or      a
                jp      z,fd_done
                dec     a
                ld      (fd_dep),a
                ld      e,a
                ld      d,0
                ld      hl,fd_plen
                add     hl,de
                ld      e,(hl)
                ld      d,0
                ld      hl,fd_path
                add     hl,de
                ld      (hl),0                  ; 경로를 원래 길이로
                ld      a,(fd_dep)
                call    fd_fibadr
                ld      de,FIB
                ld      bc,64
                ldir                            ; 위 폴더의 FIB 되살림
                call    fd_show
                jr      fd_nx
fd_dir:         ld      a,(fd_mode)
                or      a
                call    nz,tr_add
                ld      a,(fd_dep)              ; 하위 폴더로 내려감
                cp      FDDEP
                jr      nc,fd_nx
                ld      hl,fd_path
                call    strlen
                cp      40
                jr      nc,fd_nx
                ld      c,a
                push    af
                ld      a,(fd_dep)
                ld      e,a
                ld      d,0
                ld      hl,fd_plen
                add     hl,de
                pop     af
                ld      (hl),a                  ; 내려가기 전 경로 길이 기억 (그 자리에 0 을 써서 되돌림)
                ld      a,(fd_dep)
                call    fd_fibadr
                ex      de,hl
                ld      hl,FIB
                ld      bc,64
                ldir                            ; 지금 폴더의 FIB 저장
                ld      hl,fd_path
                call    strlen_hl
                ld      e,a
                ld      d,0
                ld      hl,fd_path
                add     hl,de
                dec     hl
                ld      a,(hl)
                inc     hl
                cp      '\'
                jr      z,fdd_n
                ld      (hl),'\'
                inc     hl
fdd_n:          ex      de,hl
                ld      hl,FIB+1
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,fd_dep
                inc     (hl)
                call    fd_show
                jp      fd_lvl
fd_done:        ld      a,(fd_mode)
                or      a
                jp      nz,tr_show
                call    fd_results
                ret
; HL = 문자열 → A = 길이 (HL 보존)
strlen_hl:      push    hl
                call    strlen
                pop     hl
                ret
; A = 단계 → HL = 그 단계 FIB 보관 자리
fd_fibadr:      ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; × 64
                ld      de,fd_fibs
                add     hl,de
                ret
; 진행 팝업
fd_show:        ld      hl,txt_fdsrch
                ld      (pp_t1),hl
                ld      hl,fd_path
                ld      (pp_t2),hl
                ld      hl,txt_fdesc
                ld      (pp_t3),hl
                ld      a,50
                ld      (pp_minw),a
                jp      popup_core
; ESC 가 눌렸으면 fd_stop = 1
fd_chk:         ld      ix,CHSNS
                call    bios
                ret     z
                ld      ix,CHGET
                call    bios
                cp      1Bh
                ret     nz
                ld      a,1
                ld      (fd_stop),a
                ret
; 결과 하나 더하기: 경로 + \ + 이름
fd_addres:      ld      a,(ff_n)
                cp      FDMAX
                ret     nc
                call    fd_recadr               ; HL = 자리
                ex      de,hl
                ld      hl,fd_path
                ld      b,60
fa_c:           ld      a,(hl)
                or      a
                jr      z,fa_e
                ld      (de),a
                inc     hl
                inc     de
                djnz    fa_c
fa_e:           dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,fa_n
                ld      a,'\'
                ld      (de),a
                inc     de
fa_n:           ld      hl,FIB+1
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,ff_n
                inc     (hl)
                ret
; (ff_n) 번째 자리 → HL
fd_recadr:      ld      a,(ff_n)
fd_recadr2:     ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl                   ; ×2
                add     hl,hl                   ; ×4
                add     hl,hl                   ; ×8
                ld      b,h
                ld      c,l                     ; BC = ×8
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×64
                add     hl,bc                   ; ×72
                ld      de,CPBUF
                add     hl,de
                ret
; 틀 맞추기: HL = 틀(대문자, * ?), DE = 이름 → Z = 맞음
fd_match:       ld      (fm_sp),hl              ; (임시: 별표 뒤 번지를 0 으로 쓰려고 먼저 지움)
                ld      bc,0
                ld      (fm_sp),bc
fm_l:           ld      a,(hl)
                or      a
                jr      z,fm_pe
                cp      '*'
                jr      z,fm_s
                ld      b,a
                ld      a,(de)
                or      a
                jr      z,fm_f
                ld      c,a
                ld      a,b
                cp      '?'
                jr      z,fm_a
                cp      c
                jr      nz,fm_b
fm_a:           inc     hl
                inc     de
                jr      fm_l
fm_s:           inc     hl
                ld      (fm_sp),hl
                ld      (fm_st),de
                jr      fm_l
fm_pe:          ld      a,(de)
                or      a
                ret     z
fm_b:           ld      hl,(fm_sp)
                ld      a,h
                or      l
                jr      z,fm_f
                ld      de,(fm_st)
                ld      a,(de)
                or      a
                jr      z,fm_f
                inc     de
                ld      (fm_st),de
                jr      fm_l
fm_f:           or      1
                ret
; ---- 결과 목록 화면 ----
fd_results:     ld      a,(ff_n)
                or      a
                jr      nz,fr_l
                ld      hl,txt_fdnone
                ld      de,txt_fdok
                call    popup_ask
                ret
fr_l:           xor     a
                ld      (fd_sel),a
                ld      (fd_top),a
fr_d:           call    clear_screen            ; (처음 한 번만: 이름표·안내 줄. ↑↓ 는 바뀐 줄만 다시 그림)
                ld      hl,txt_fdtit
                xor     a
                call    qd_line
                ld      hl,txt_fdfoot
                ld      a,23
                call    qd_line
                call    fr_all
                jr      fr_k
; 결과 목록 20 줄 (fd_top 부터)
fr_all:         ld      a,(fd_top)
                ld      (ff_i),a
                xor     a
                ld      (fd_r),a
fr_row:         ld      a,(ff_i)
                ld      b,a
                ld      a,(ff_n)
                cp      b
                ret     z
                ret     c
                ld      a,(fd_r)
                cp      20
                ret     nc
                call    fr_rw
                ld      hl,ff_i
                inc     (hl)
                ld      hl,fd_r
                inc     (hl)
                jr      fr_row
; A = 결과 번호 → 그 줄만 그림
fr_one:         ld      (ff_i),a
                ld      hl,fd_top
                sub     (hl)
                ld      (fd_r),a
; ff_i 번 결과를 fd_r 번째 줄에
fr_rw:          ld      a,(ff_i)
                call    fd_recadr2
                ld      de,qd_tmp+1
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,' '
                ld      hl,fd_sel
                ld      b,a
                ld      a,(ff_i)
                cp      (hl)
                ld      a,b
                jr      nz,fr_m
                ld      a,'>'
fr_m:           ld      (qd_tmp),a
                ld      hl,qd_tmp
                ld      a,(fd_r)
                add     a,2
                jp      qd_line
fr_k:           call    wait_key
                cp      1Bh
                ret     z
                cp      K_DOWN
                jr      nz,fr_u
                ld      a,(fd_sel)
                ld      (fd_old),a
                inc     a
                ld      b,a
                ld      a,(ff_n)
                cp      b
                jr      z,fr_k
                ld      a,b
                ld      (fd_sel),a
                jr      fr_s
fr_u:           cp      K_UP
                jr      nz,fr_r
                ld      a,(fd_sel)
                or      a
                jr      z,fr_k
                ld      (fd_old),a
                dec     a
                ld      (fd_sel),a
fr_s:           ld      a,(fd_top)
                ld      (fd_ot),a
                ld      a,(fd_sel)              ; 화면 밖이면 위/아래로 맞춤
                ld      hl,fd_top
                cp      (hl)
                jr      nc,fr_s2
                ld      (hl),a
fr_s2:          ld      a,(fd_top)
                add     a,20
                ld      b,a
                ld      a,(fd_sel)
                cp      b
                jr      c,fr_nu
                ld      a,(fd_sel)
                sub     19
                ld      (fd_top),a
fr_nu:          ld      a,(fd_ot)               ; 목록이 넘어갔으면 목록 줄들만, 아니면 예전 줄·새 줄 두 줄만
                ld      hl,fd_top
                cp      (hl)
                jr      z,fr_2
                call    fr_all
                jr      fr_k
fr_2:           ld      a,(fd_old)
                call    fr_one
                ld      a,(fd_sel)
                call    fr_one
                jr      fr_k
fr_r:           cp      0Dh
                jr      nz,fr_k
                ld      a,(fd_sel)
                call    fd_recadr2              ; HL = 고른 결과 "경로\이름"
                jp      fd_go
; HL = 경로\이름 → 활성 패널을 그 폴더로, 커서를 그 파일에
fd_go:          ld      (fd_rec),hl
                ld      d,h
                ld      e,l
fg_s:           ld      a,(hl)
                or      a
                jr      z,fg_e
                cp      '\'
                jr      nz,fg_n
                ld      d,h
                ld      e,l
fg_n:           inc     hl
                jr      fg_s
fg_e:           ld      (fd_bs),de              ; 마지막 '\' 자리
                call    cur_ix
                call    path_save
                ld      hl,(fd_bs)
                ld      de,(fd_rec)
                or      a
                sbc     hl,de                   ; HL = 폴더 부분 길이
                ld      a,l
                cp      2
                jr      nz,fg_c
                inc     l                       ; 루트 "A:\" 는 '\' 까지
fg_c:           ld      c,l
                ld      b,0
                push    bc
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ex      de,hl
                pop     bc
                ld      hl,(fd_rec)
                ldir
                xor     a
                ld      (de),a
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                jr      z,fg_ok
                call    explain_err
                call    path_restore
                jp      panel_load
fg_ok:          ld      hl,(fd_bs)
                inc     hl
                ld      de,upname
                call    strcpy_nul
                call    cur_ix
                jp      find_exact
txt_fdq:        db      "Find file (* ? allowed)",0
txt_fdsrch:     db      "Searching...",0
txt_fdesc:      db      "[Esc] Stop",0
txt_fdnone:     db      "No files found",0
txt_fdok:       db      "[Enter] OK",0
txt_fdtit:      db      "Find results   Up/Down select   Enter go to folder   Esc close",0
txt_fdfoot:     db      "(only the first 28 matches are listed; names are matched by 8.3 name)",0
fd_pat:         ds      28
fd_pp:          dw      0
fd_path:        ds      64
fd_plen:        ds      8
fd_fibs         equ     LVSTK                   ; (복사용 단계 저장소를 빌려 씀: 찾기 중에는 복사를 하지 않음)
fd_dep:         db      0
fd_stop:        db      0
ff_n:           db      0
fd_sel:         db      0
fd_top:         db      0
ff_i:           db      0
fd_r:           db      0
fd_old:         db      0
fd_ot:          db      0
fd_rec:         dw      0
fd_bs:          dw      0
fm_sp:          dw      0
fm_st:          dw      0

; ======================================================================
; Ctrl+W 트리 보기: 활성 패널 드라이브의 루트부터 폴더만 모아(깊이 6, 140 개까지) 들여쓰기 목록으로 보여 줌.
;  위/아래 고르기, RETURN = 그 폴더로 이동, ESC 닫기.  (찾기와 같은 하위 폴더 훑기 코드를 씀: fd_mode = 1)
; ======================================================================
TRMAX           equ     140
TRREC           equ     14
do_tree:        call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      a,(hl)                  ; 루트 "X:\" 만
                ld      (fd_path),a
                ld      a,':'
                ld      (fd_path+1),a
                ld      a,'\'
                ld      (fd_path+2),a
                xor     a
                ld      (fd_path+3),a
                ld      (fd_dep),a
                ld      (fd_stop),a
                inc     a
                ld      (fd_mode),a
                xor     a
                ld      (tr_n),a
                ld      hl,fd_path              ; 첫 항목 = 루트 (깊이 0, 이름 "X:\")
                ld      a,(tr_n)
                call    tr_adr
                xor     a
                ld      (hl),a
                inc     hl
                ex      de,hl
                ld      hl,fd_path
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,1
                ld      (tr_n),a
                ld      hl,txt_trread
                ld      (pp_t1),hl
                ld      hl,0
                ld      (pp_t2),hl
                ld      hl,txt_fdesc
                ld      (pp_t3),hl
                ld      a,30
                ld      (pp_minw),a
                call    popup_core
                jp      fd_lvl
; 폴더 하나 더하기 (깊이 = fd_dep + 1, 이름 = FIB+1)
tr_add:         ld      a,(tr_n)
                cp      TRMAX
                ret     nc
                call    tr_adr
                ld      a,(fd_dep)
                inc     a
                ld      (hl),a
                inc     hl
                ex      de,hl
                ld      hl,FIB+1
                ld      b,12
ta_c:           ld      a,(hl)
                or      a
                jr      z,ta_e
                ld      (de),a
                inc     hl
                inc     de
                djnz    ta_c
ta_e:           xor     a
                ld      (de),a
                ld      hl,tr_n
                inc     (hl)
                ret
; A = 번호 → HL = 항목 자리 (CPBUF + 번호 × 14)
tr_adr:         ld      l,a
                ld      h,0
                add     hl,hl
                ld      b,h
                ld      c,l                     ; BC = ×2
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×16
                or      a
                sbc     hl,bc                   ; ×14
                ld      de,CPBUF
                add     hl,de
                ret
; 번호 (tr_i) 의 앞에 붙일 선 글자를 DE 에 씀 (DE 는 끝으로)
tr_prefix:      push    de
                ld      a,(tr_i)
                call    tr_adr
                ld      a,(hl)
                pop     de
                ld      (tr_d2),a
                or      a
                ret     z
                ld      c,1
tp_l:           ld      a,(tr_d2)
                cp      c
                jr      z,tp_last
                push    bc
                push    de
                call    tr_hasnext
                pop     de
                pop     bc
                or      a
                ld      a,CH_VBAR
                jr      nz,tp_w
                ld      a,' '
tp_w:           ld      (de),a
                inc     de
                ld      a,' '
                ld      (de),a
                inc     de
                inc     c
                jr      tp_l
tp_last:        push    de
                call    tr_hasnext
                pop     de
                or      a
                ld      a,CH_TEE
                jr      nz,tp_t
                ld      a,CH_BL
tp_t:           ld      (de),a
                inc     de
                ld      a,CH_HBAR
                ld      (de),a
                inc     de
                ret
; (tr_i) 번호 뒤에서 깊이 C 의 형제가 더 있는가 → A = 1 / 0
tr_hasnext:     ld      a,(tr_i)
                inc     a
                ld      b,a
th_l:           ld      a,(tr_n)
                cp      b
                jr      z,th_no
                jr      c,th_no
                ld      a,b
                push    bc
                call    tr_adr
                ld      a,(hl)
                pop     bc
                cp      c
                jr      z,th_yes
                jr      c,th_no
                inc     b
                jr      th_l
th_yes:         ld      a,1
                ret
th_no:          xor     a
                ret
tr_show:        xor     a
                ld      (tr_sel),a
                ld      (tr_top),a
trs_d:          call    clear_screen            ; (처음 한 번만: 이름표·안내 줄. ↑↓ 는 바뀐 줄만 다시 그림)
                ld      hl,txt_trtit
                xor     a
                call    qd_line
                call    trs_all
                jr      trs_k
; 트리 22 줄 (tr_top 부터)
trs_all:        ld      a,(tr_top)
                ld      (tr_i),a
                xor     a
                ld      (tr_r),a
trs_r:          ld      a,(tr_i)
                ld      b,a
                ld      a,(tr_n)
                cp      b
                ret     z
                ret     c
                ld      a,(tr_r)
                cp      22
                ret     nc
                call    trs_rw
                ld      hl,tr_i
                inc     (hl)
                ld      hl,tr_r
                inc     (hl)
                jr      trs_r
; A = 항목 번호 → 그 줄만 그림
trs_one:        ld      (tr_i),a
                ld      hl,tr_top
                sub     (hl)
                ld      (tr_r),a
; tr_i 번 항목을 tr_r 번째 줄에
trs_rw:         ld      de,qd_tmp+1
                call    tr_prefix               ; 선: │ ├─ └─
                ld      a,(tr_i)
                push    de
                call    tr_adr
                pop     de
                inc     hl
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,' '
                ld      hl,tr_sel
                ld      b,a
                ld      a,(tr_i)
                cp      (hl)
                ld      a,b
                jr      nz,trs_m
                ld      a,'>'
trs_m:          ld      (qd_tmp),a
                ld      hl,qd_tmp
                ld      a,(tr_r)
                inc     a
                jp      qd_line
trs_k:          call    wait_key
                cp      1Bh
                ret     z
                cp      K_DOWN
                jr      nz,trs_u
                ld      a,(tr_sel)
                ld      (tr_old),a
                inc     a
                ld      b,a
                ld      a,(tr_n)
                cp      b
                jr      z,trs_k
                ld      a,b
                ld      (tr_sel),a
                jr      trs_s
trs_u:          cp      K_UP
                jr      nz,trs_r2
                ld      a,(tr_sel)
                or      a
                jr      z,trs_k
                ld      (tr_old),a
                dec     a
                ld      (tr_sel),a
trs_s:          ld      a,(tr_top)
                ld      (tr_ot),a
                ld      a,(tr_sel)
                ld      hl,tr_top
                cp      (hl)
                jr      nc,trs_s2
                ld      (hl),a
trs_s2:         ld      a,(tr_top)
                add     a,22
                ld      b,a
                ld      a,(tr_sel)
                cp      b
                jr      c,trs_nu
                ld      a,(tr_sel)
                sub     21
                ld      (tr_top),a
trs_nu:         ld      a,(tr_ot)               ; 목록이 넘어갔으면 목록 줄들만, 아니면 예전 줄·새 줄 두 줄만
                ld      hl,tr_top
                cp      (hl)
                jr      z,trs_2
                call    trs_all
                jr      trs_k
trs_2:          ld      a,(tr_old)
                call    trs_one
                ld      a,(tr_sel)
                call    trs_one
                jr      trs_k
trs_r2:         cp      0Dh
                jr      nz,trs_k
                jp      tr_go
; 고른 항목의 경로를 만들어 활성 패널을 그 폴더로
tr_go:          ld      a,(tr_sel)
                call    tr_adr
                ld      a,(hl)
                ld      (tr_d),a                ; 깊이
                ld      c,a
                ld      a,(tr_sel)
                ld      b,a                     ; B = 거꾸로 훑을 번호
tg_ch:          ld      a,c
                or      a
                jr      z,tg_b
                push    bc
                ld      a,b
                call    tr_adr
                ld      a,(hl)
                pop     bc
                cp      c
                jr      nz,tg_n
                ld      e,c
                ld      d,0
                ld      hl,tr_ch
                add     hl,de
                ld      (hl),b
                dec     c
tg_n:           dec     b
                jr      tg_ch
tg_b:           ld      a,0                     ; 경로 = 루트 + 이름들
                call    tr_adr
                inc     hl
                ld      de,tr_pb
                call    copy_z
                xor     a
                ld      (de),a
                ld      c,1
tg_l:           ld      a,(tr_d)
                cp      c
                jr      c,tg_x
                push    bc
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,tg_a
                ld      a,'\'
                ld      (de),a
                inc     de
tg_a:           pop     bc
                push    bc
                ld      b,0
                ld      hl,tr_ch
                add     hl,bc
                ld      a,(hl)
                push    de
                call    tr_adr
                pop     de
                inc     hl
                call    copy_z
                xor     a
                ld      (de),a
                pop     bc
                inc     c
                jr      tg_l
tg_x:           call    cur_ix
                call    path_save
                push    ix
                pop     de
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                ld      hl,tr_pb
                ld      bc,64
                ldir
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                ret     z
                call    explain_err
                call    path_restore
                jp      panel_load
txt_trtit:      db      "Tree   Up/Down select   Enter go to folder   Esc close",0
txt_trread:     db      "Reading folders...",0
fd_mode:        db      0
tr_n:           db      0
tr_sel:         db      0
tr_top:         db      0
tr_i:           db      0
tr_old:         db      0
tr_ot:          db      0
tr_r:           db      0
tr_d:           db      0
tr_d2:          db      0
tr_ch:          ds      8
tr_pb:          ds      64

