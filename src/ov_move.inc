; 만든이: KimTaeWoo
; X86FC 이동(SELECT) - SPDX-License-Identifier: GPL-3.0-or-later
; (0.12.11) 본체에서 오버레이 5 로 옮김: 본체 메모리 확보 (ovl.cfg 5 ov_move.inc do_move)

; ======================================================================
; 이동 (SELECT 키): 고른 항목(없으면 커서 항목)을 활성 패널 폴더 → 반대 패널 폴더로.
; 같은 드라이브: 원본의 긴 이름 조각을 모아 두고 DOS _MOVE 로 8.3 항목을 옮긴 뒤 조각을 다시 씀
; (같은 이름의 파일이 있으면 덮어쓸지 물음). 폴더 겹치기·8.3 겹침·다른 드라이브는
; 복사 후 원본 삭제 (건너뛴 것이 있으면 원본 폴더는 지우지 않음)
; ======================================================================
do_move:        call    cur_ix
                ld      (cp_src),ix
                ld      a,(act)
                xor     1
                call    panel_ix
                ld      (cp_dst),ix
                ld      ix,(cp_src)
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      hl,(cp_src)             ; 같은 디렉터리인가
                ld      de,PS_PATH
                add     hl,de
                ex      de,hl
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                call    str_eq
                jr      nz,mvd_p1
                ld      hl,txt_same
                ld      (msg_p),hl
                ret
mvd_p1:         ld      hl,(cp_src)             ; 같은 드라이브인가
                ld      de,PS_PATH
                add     hl,de
                ld      c,(hl)
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      a,(hl)
                cp      c
                ld      a,0
                jr      nz,mvd_sd
                inc     a
mvd_sd:         ld      (mv_same),a
                or      a
                jr      nz,mvd_t
                call    cp_setbuf               ; 다른 드라이브: 복사 버퍼가 필요
                jr      z,mvd_t
                ld      hl,txt_nomem2
                ld      (msg_p),hl
                ret
mvd_t:          ld      ix,(cp_src)
                xor     a
                ld      (dl_one),a
                ld      a,(ix+PS_SEL)
                or      (ix+PS_SEL+1)
                jr      nz,mvd_many
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    is_dotdot
                ret     z
                ld      (dl_ent),hl
                ld      a,1
                ld      (dl_one),a
                ld      de,ne_a
                call    get_ds
                ld      de,errbuf               ; "Move NAME? Y/N"
                ld      hl,txt_mv
                call    copy_z
                ld      hl,ne_a
                ld      b,22
                call    copy_lim
                ld      hl,txt_yn
                call    copy_z
                jr      mvd_ask
mvd_many:       ld      de,errbuf               ; "Move N item(s)? Y/N"
                ld      hl,txt_mv
                call    copy_z
                ld      l,(ix+PS_SEL)
                ld      h,(ix+PS_SEL+1)
                call    put_dec16
                ld      hl,txt_items2
                call    copy_z
                ld      hl,txt_yn
                call    copy_z
mvd_ask:        call    ask_end
                cp      'Y'
                jr      z,mvd_go
                ld      hl,0
                ld      (msg_p),hl
                ret
mvd_go:         xor     a
                ld      (cp_abort),a
                ld      (ow_all),a
                ld      (dl_all),a
                ld      (dq_n),a
                ld      (cp_skip),a
                ld      hl,0
                ld      (cp_cnt),hl
                ld      (mv_n),hl
                ld      (msg_p),hl
                ld      a,(dl_one)
                or      a
                jr      z,mvd_sel
                ld      hl,(dl_ent)
                call    mvd_item
                jr      mvd_phb
mvd_sel:        call    selmap_build
                ld      hl,0
                ld      (cp_i),hl
mvd_l:          ld      a,(cp_abort)
                or      a
                jr      nz,mvd_phb
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,mvd_phb
                ld      hl,(cp_i)
                call    selmap_test
                jr      z,mvd_n
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                call    ptr_get
                call    mvd_item
mvd_n:          ld      hl,(cp_i)
                inc     hl
                ld      (cp_i),hl
                jr      mvd_l
mvd_phb:        xor     a                       ; 2) 모아 둔 폴더들 (이름으로 다시 찾아 처리)
                ld      (dq_i),a
mvd_pb:         ld      a,(cp_abort)
                or      a
                jr      nz,mvd_fin
                ld      a,(dq_i)
                ld      hl,dq_n
                cp      (hl)
                jr      nc,mvd_fin
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,de
                add     hl,de
                add     hl,de
                ld      de,dq
                add     hl,de
                ex      de,hl
                ld      ix,(cp_src)
                call    find_n11
                ld      a,h
                or      l
                jr      z,mvd_pn
                call    mv_one
mvd_pn:         ld      a,(dq_i)
                inc     a
                ld      (dq_i),a
                jr      mvd_pb
mvd_fin:        ld      ix,(cp_dst)             ; 반대 패널을 다시 읽음
                call    panel_load
                ld      ix,(cp_src)
                call    panel_load
                ld      hl,(msg_p)
                ld      a,h
                or      l
                ret     nz
                ld      de,errbuf
                ld      hl,txt_moved
                call    copy_z
                ld      hl,(mv_n)
                call    put_dec16
                ld      hl,txt_items2
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      (msg_p),hl
                ret

; HL = 항목: 파일이면 처리, 폴더면 이름을 큐에
mvd_item:       push    hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                pop     hl
                jp      z,mv_one
                jp      dd_queue

; 복사 버퍼 크기(cp_bufsz) 계산 → Z = 충분 (512 의 배수, 최소 2KB)
; ======================================================================
; D 단계: 프로그램 실행과 되돌아오기 (0.12.6 에서 방식 바뀜)
;  - 실행 직전에 두 패널의 폴더·커서 항목·활성 패널을 환경 변수 X86FC 에 저장, 실행할 명령 글은 X86FC_C, 끝난 뒤 쉴지는 X86FC_P
;  - X86FC.COM 폴더에 고정 내용의 배치 파일 X86FCRUN.BAT 를 쓰고(%X86FC_C% / %X86FC_P% / X86FC.COM 전체 경로),
;    현재 드라이브·폴더를 활성 패널로 맞추고 화면을 되돌린 뒤, 키 버퍼에 ESC + 배치 이름 + RETURN 을 넣고 _TERM 으로 완전히 끝남
;  - COMMAND2 가 배치를 실행 → 프로그램이 돌고(TPA 를 마음대로 씀) → 마지막 줄이 X86FC 를 다시 띄움 → 시작할 때 환경 변수를 읽어 같은 줄로 복귀
;    (배치 내용이 늘 같아서 실행 중인 배치를 다시 써도 안전 — 환경 변수로 명령만 바꿈)
; ======================================================================

; HL = 원본 항목 하나 이동
mv_one:         ld      (cp_ent),hl
                ld      a,(mv_same)
                or      a
                jp      z,mv_cross
                ld      hl,(cp_ent)
                call    disp_name
                ld      hl,(cp_src)             ; 원본 경로
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      hl,(cp_ent)             ; 폴더를 옮길 수 있는가
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      z,mv_nd1
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                ld      hl,srcpath
                call    path_inside
                jr      nz,mv_nd1
                ld      hl,txt_mvself
                ld      (msg_p),hl
                ld      hl,cp_skip
                inc     (hl)
                ret
mv_nd1:         ld      hl,(cp_ent)             ; 별칭 = 원본 8.3 이름
                ld      de,cp_alias
                ld      bc,11
                ldir
                ld      ix,(cp_dst)
                ld      de,cp_alias
                call    find_n11
                ld      a,h
                or      l
                jp      z,mv_lfn
                ld      (cp_hit),hl
                ex      de,hl
                ld      hl,(cp_ent)
                call    names_eq
                jp      nz,mv_cross             ; 8.3 이 겹침: 복사 쪽에서 새 별칭 (그 다음 원본 삭제)
                ld      hl,(cp_ent)             ; 같은 이름: 파일이면 덮어쓸지 물음 (폴더이면 합쳐야 하므로 복사 쪽)
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jp      nz,mv_cross
                ld      hl,(cp_hit)
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jp      nz,mv_cross
                ld      a,(ow_all)
                or      a
                jr      nz,mv_del
                ld      hl,(cp_ent)
                ld      de,ne_a
                call    get_ds
                call    ask_over
                cp      'Y'
                jr      z,mv_del
                cp      'A'
                jr      nz,mv_no
                ld      a,1
                ld      (ow_all),a
                jr      mv_del
mv_no:          cp      'N'
                jr      nz,mv_esc
                ld      hl,cp_skip
                inc     (hl)
                ret
mv_esc:         ld      a,1
                ld      (cp_abort),a
                ret
mv_del:         ld      hl,cp_alias             ; 덮어쓸 파일(과 긴 이름 조각)을 먼저 지움
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,dstpath
                call    mkpath
                ld      de,dstpath
                ld      c,F_DELETE
                call    BDOS
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,cp_alias
                call    lf_erase
                jp      mv_go
mv_lfn:
                ld      hl,(cp_ent)             ; 8.3 은 다르지만 긴 이름이 같은 항목이 대상에 있으면 복사 쪽
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      a,d
                or      e
                jp      z,mv_go
                ex      de,hl
                ld      ix,(cp_dst)
                call    find_lfn
                ld      a,h
                or      l
                jp      z,mv_go
                jp      mv_cross
mv_go:
                xor     a                       ; 긴 이름 조각을 원본 폴더에서 모아 둠
                ld      (lf_found),a
                ld      (lf_rawn),a
                ld      hl,(cp_ent)
                ld      de,EN_LFN
                add     hl,de
                ld      a,(hl)
                inc     hl
                or      (hl)
                jr      z,mv_dos
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,(cp_ent)
                call    lf_capture
mv_dos:         ld      hl,(cp_dst)             ; DOS 로 8.3 항목 이동: DE = 원본 경로, HL = 대상 폴더
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                ld      c,F_MOVE
                call    BDOS
                or      a
                jr      z,mv_ok
                ld      b,a
                ld      a,b
                call    explain_err
                ld      a,1
                ld      (cp_abort),a
                ret
mv_ok:          ld      hl,(cp_ent)             ; 원본 폴더에서 옛 조각을 지움
                ld      de,EN_LFN
                add     hl,de
                ld      a,(hl)
                inc     hl
                or      (hl)
                jr      z,mv_inst
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,(cp_ent)
                call    lf_erase
mv_inst:        ld      a,(lf_found)            ; 대상 폴더에 조각을 다시 씀
                or      a
                jr      z,mv_end
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,cp_alias
                call    lf_install
                ld      a,(in_placed)
                or      a
                jr      nz,mv_end
                ld      hl,txt_nolfn
                ld      (msg_p),hl
mv_end:         call    append_dst
                ld      hl,(mv_n)
                inc     hl
                ld      (mv_n),hl
                ret

; 복사한 뒤 원본을 지워서 옮김 (다른 드라이브, 폴더 겹치기, 8.3 겹침)
mv_cross:
                ld      hl,(cp_ent)
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      nz,mvx_dir
                ld      hl,(cp_cnt)             ; 파일: 복사가 잘 되었으면 원본 지움
                ld      (mv_c0),hl
                ld      hl,(cp_ent)
                call    cp_one
                ld      a,(cp_abort)
                or      a
                ret     nz
                ld      hl,(cp_cnt)
                ld      de,(mv_c0)
                or      a
                sbc     hl,de
                ret     z                       ; 복사 안 함(건너뜀): 원본 그대로
                ld      hl,(cp_ent)
                call    del_file
                ld      a,(cp_abort)
                or      a
                ret     nz
                ld      hl,(mv_n)
                inc     hl
                ld      (mv_n),hl
                ret
mvx_dir:        ld      hl,(cp_ent)             ; 폴더: 이름을 저장해 두고 차례로 복사
                ld      de,mv_nm
                ld      bc,11
                ldir
                xor     a
                ld      (cp_skip),a
                ld      hl,(cp_ent)
                call    cp_one
                ld      a,(cp_abort)
                or      a
                ret     nz
                ld      a,(cp_skip)
                or      a
                jr      z,mvx_del
                ld      hl,txt_mvkeep           ; 건너뛴 것이 있어서 원본 폴더는 지우지 않음
                ld      (msg_p),hl
                ret
mvx_del:        ld      ix,(cp_src)             ; 복사하는 동안 목록이 다시 읽혔으므로 이름으로 원본 폴더를 다시 찾음
                ld      de,mv_nm
                call    find_n11
                ld      a,h
                or      l
                ret     z
                call    del_tree
                ld      a,(cp_abort)
                or      a
                ret     nz
                ld      hl,(mv_n)
                inc     hl
                ld      (mv_n),hl
                ret


; ---- (0.12.14) 옛 파일 이행: X86FC.SET(SCREEN·SORT 줄) · X86FC.MNU(MENU 줄) · X86FC.QD(경로 한 줄씩) · X86FC.HIS(104바이트 x 4) 를
;      INI 의 SCREEN·SORT·MENU·QDIR·HIST 줄로 옮기고 지움 (진입 1, 본체 mig_chk 가 부름) ----
mig_old:        ld      hl,x_set
                ld      (mg_ext),hl
                ld      hl,k_scr
                call    mg_key
                ld      hl,x_set
                ld      (mg_ext),hl
                ld      hl,k_srt
                call    mg_key
                ld      hl,x_mnu
                ld      (mg_ext),hl
                ld      hl,k_menu
                call    mg_key
                ld      hl,x_qd
                ld      (mg_ext),hl
                ld      hl,k_qdir
                ld      de,gen_mgq
                call    mg_run
                ld      hl,x_his
                ld      (mg_ext),hl
                ld      hl,k_hist
                ld      de,gen_mgh
                call    mg_run
                ld      a,(mg_bad)              ; 이행 끝난 SET 지움 (SCREEN·SORT 둘 다 옮긴 뒤, 실패 없을 때만)
                or      a
                ret     nz
                ld      hl,x_set
                call    mg_del
                ret
mg_key:         ld      de,gen_mgk
; HL = 키 (iw_key),  DE = 만들기 루틴,  mg_ext = 옛 파일 확장자.  옛 파일이 있으면 INI 로 옮기고 (MNU·QD·HIS 는) 지움
mg_run:         ld      (iw_key),hl
                ld      (mg_gn),de
                ld      hl,(mg_ext)             ; 옛 파일이 있나
                call    xpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      c,F_CLOSE
                call    BDOS
                ld      hl,iw_mkey
                ld      de,(mg_gn)
                call    ini_wr
                jr      z,mgr_ok
                ld      a,1                     ; 실패하면 옛 파일을 남김
                ld      (mg_bad),a
                ret
mgr_ok:
                ld      hl,(mg_ext)
                ld      de,x_set
                or      a
                sbc     hl,de
                ret     z                       ; SET 은 SORT 까지 옮긴 뒤에 지움
                ld      hl,(mg_ext)
mg_del:         call    xpath
                ld      c,F_DELETE
                jp      BDOS
mg_ext:         dw      0
mg_bad:         db      0
mg_gn:          dw      0
k_scr:          db      "screen",0
k_srt:          db      "sort",0
; 옛 파일에서 키(iw_key)가 맞는 줄을 그대로 씀
gen_mgk:        ld      hl,(mg_ext)
                call    xpath
                call    il_openf
                ret     nz
gk_l:           call    il_next
                jr      z,gk_e
                ld      de,(iw_key)
                call    il_key
                jr      nz,gk_l
                ld      hl,ILBUF
                call    iw_put
                jr      gk_l
gk_e:           jp      il_close
; QD: 줄마다 경로 -> "QDIR = 경로"
gen_mgq:        ld      hl,(mg_ext)
                call    xpath
                call    il_openf
                ret     nz
gq2_l:          call    il_next
                jr      z,gk_e
                ld      a,(ILBUF)
                or      a
                jr      z,gq2_l
                ld      de,CMDBUF
                ld      hl,txt_qdk
                call    copy_z
                ld      hl,ILBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                call    iw_put
                jr      gq2_l
; HIS: 104바이트 x 4 -> "HIST = 글" (빈 칸 빼고)
gen_mgh:        ld      hl,(mg_ext)
                call    xpath
                call    il_openf
                ret     nz
                ld      hl,CPBUF
                ld      de,CPBUF+1
                ld      bc,415
                ld      (hl),0
                ldir
                ld      a,(ir_h)
                ld      b,a
                ld      de,CPBUF
                ld      hl,416
                ld      c,F_READ
                call    BDOS
                call    il_close
                ld      hl,CPBUF
                ld      (hi_p),hl
                ld      b,4
gm_l:           push    bc
                ld      hl,(hi_p)
                ld      a,(hl)
                or      a
                jr      z,gm_n
                ld      de,CMDBUF
                push    hl
                ld      hl,txt_hik
                call    copy_z
                pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                call    iw_put
gm_n:           ld      hl,(hi_p)
                ld      de,104
                add     hl,de
                ld      (hi_p),hl
                pop     bc
                djnz    gm_l
                ret


; ---- (0.12.15) 본체에서 옮겨 온 것: F9 드라이브 풀다운 (진입 2), F8 정렬 풀다운 (진입 3) ----
; 드라이브 고르기(^D): A~H 를 눌러 드라이브 루트로
drive_prompt:   call    drv_scan                ; 실제로 쓸 수 있는 드라이브만 찾아 단추 글을 만듦
                call    drv_menu                ; 풀다운 목록 (작동하는 드라이브만) → A = 글자, 0 = 취소
                push    af
                call    draw_all
                pop     af
dp_up:          ld      c,a                     ; 목록에 있는 글자만 받음
                ld      hl,drv_list
dpk_f:           ld      a,(hl)
                or      a
                ret     z
                cp      c
                jr      z,dpk_ok
                inc     hl
                jr      dpk_f
dpk_ok:          ld      a,c
                ld      (dl_key),a              ; (path_save 가 BC 를 쓰므로 드라이브 글자는 따로 둠)
                call    cur_ix
                call    path_save
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      a,(dl_key)
                ld      (hl),a
                inc     hl
                ld      (hl),':'
                inc     hl
                ld      (hl),'\'
                inc     hl
                ld      (hl),0
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                ret     z
                call    explain_err
                call    path_restore
                jp      panel_load

; 쓸 수 있는 드라이브 찾기: DRVTBL(FB21h) 의 드라이브 수만큼 A: 부터 "X:\*.*" 를 찾아 봐서
;   파일이 있거나 "더 없음" 이면 작동하는 드라이브로 봄 (디스크 없음·잘못된 드라이브는 뺌) → drv_list(글자들), drv_btn(단추 글)
drv_scan:       ld      hl,drv_list
                ld      (drv_p),hl
                ld      c,18h                   ; _LOGIN: 드라이브 비트표 (bit0 = A:) — 디스크를 건드리지 않음 (안 건드려야 없는 디스크 때문에 멈추지 않음)
                call    BDOS
                ld      a,h
                or      l
                jr      nz,dsc_m
                ld      hl,0FB21h              ; (표가 비면) DRVTBL: 4 칸 x (드라이브 수, 슬롯) 의 드라이브 수 합
                ld      b,4
                ld      c,0
dsc_t:          ld      a,(hl)
                add     a,c
                ld      c,a
                inc     hl
                inc     hl
                djnz    dsc_t
                ld      a,c
                cp      9
                jr      c,dsc_c
                ld      a,8
dsc_c:          ld      b,a
                ld      a,0
                or      b
                jr      z,dsc_d
                xor     a
dsc_s:          scf
                rla                             ; A = (1 << 드라이브 수) - 1
                djnz    dsc_s
                ld      l,a
                ld      h,0
dsc_m:          ld      b,16                    ; A: ~ P: (Nextor 는 8 개를 넘음)
                ld      e,'A'
dsc_l:          srl     h
                rr      l
                jr      nc,dsc_no
                push    hl
                ld      hl,(drv_p)
                ld      (hl),e
                inc     hl
                ld      (drv_p),hl
                pop     hl
dsc_no:         inc     e
                djnz    dsc_l
dsc_d:          ld      hl,(drv_p)
                ld      (hl),0
                ret
; 드라이브 풀다운: 활성 패널 왼쪽 위(목록 첫 줄)부터 " A:" 줄들.  ↑↓ 고르기, RETURN 선택, 글자 = 바로 선택, ESC 취소
drv_menu:       ld      hl,drv_list
                call    strlen
                or      a
                ret     z
                cp      15
                jr      c,drm_cap
                ld      a,14                    ; 풀다운은 14 줄까지 (그 밖은 글자를 쳐서 고름)
drm_cap:        ld      (drm_n),a
                ld      a,LIST_Y                ; 목록 줄들의 반전을 끔 (풀다운과 겹쳐 보이지 않게)
                ld      (drm_i),a
drm_h:          ld      a,(act)
                ld      b,a
                ld      a,(drm_i)
                ld      c,0
                call    hl_set
                ld      a,(drm_i)
                inc     a
                ld      (drm_i),a
                ld      hl,v_rrule
                cp      (hl)
                jr      c,drm_h
                xor     a
                ld      (drm_sel),a
                call    cur_ix
                ld      c,(ix+PS_PATH)          ; 지금 드라이브에 커서
                ld      hl,drv_list
                ld      b,0
drm_f:          ld      a,(hl)
                or      a
                jp      z,drm_draw
                cp      c
                jr      nz,drm_nx
                ld      a,b
                ld      (drm_sel),a
                jp      drm_draw
drm_nx:         inc     hl
                inc     b
                jr      drm_f
drm_draw:       xor     a
                ld      (drm_i),a
drm_l:          ld      a,(drm_n)               ; 줄 0 = 위 테두리, 1~n = 드라이브, n+1 = 아래 테두리
                inc     a
                ld      hl,drm_i
                cp      (hl)
                jp      c,drm_k
                ld      a,(hl)
                or      a
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      z,drm_b
                ld      a,(drm_n)
                inc     a
                cp      (hl)
                ld      c,CH_BL
                ld      e,CH_BR
                jr      z,drm_b
                ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
drm_b:          ld      hl,qd_tmp
                ld      (hl),c
                inc     hl
                ld      b,11
drm_p:          ld      (hl),d
                inc     hl
                djnz    drm_p
                ld      (hl),e
                ld      a,(drm_i)               ; 드라이브 줄: " > A:" (고른 줄에 >)
                or      a
                jr      z,drm_w
                ld      hl,drm_n
                cp      (hl)
                jr      z,drm_it
                jr      c,drm_it
                jr      drm_w
drm_it:         dec     a
                ld      e,a
                ld      d,0
                ld      hl,drv_list
                add     hl,de
                ld      c,(hl)
                ld      hl,drm_sel
                cp      (hl)
                ld      a,' '
                jr      nz,drm_m
                ld      a,'>'
drm_m:          ld      (qd_tmp+2),a
                ld      a,c
                ld      (qd_tmp+4),a
                ld      a,':'
                ld      (qd_tmp+5),a
drm_w:          ld      a,(drm_i)
                add     a,LIST_Y
                call    row_addr
                ld      de,1
                ld      a,(act)
                or      a
                jr      z,drm_c
                ld      de,41
drm_c:          add     hl,de
                ld      de,qd_tmp
                ld      b,13
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ld      a,(drm_i)
                inc     a
                ld      (drm_i),a
                jp      drm_l
drm_k:          call    wait_key
                cp      1Bh
                jr      z,drm_esc
                cp      0Dh
                jr      z,drm_ok
                cp      K_UP
                jr      nz,drm_dn
                ld      a,(drm_sel)
                or      a
                jp      z,drm_draw
                dec     a
                ld      (drm_sel),a
                jp      drm_draw
drm_dn:         cp      K_DOWN
                jr      nz,drm_ch
                ld      a,(drm_sel)
                inc     a
                ld      hl,drm_n
                cp      (hl)
                jp      nc,drm_draw
                ld      (drm_sel),a
                jp      drm_draw
drm_ch:         cp      'a'
                jr      c,drm_u
                cp      'z'+1
                jr      nc,drm_u
                and     0DFh
drm_u:          ld      c,a
                ld      hl,drv_list
drm_s:          ld      a,(hl)
                or      a
                jp      z,drm_draw
                cp      c
                jr      z,drm_o2
                inc     hl
                jr      drm_s
drm_o2:         ld      a,c
                ret
drm_ok:         ld      a,(drm_sel)
                ld      e,a
                ld      d,0
                ld      hl,drv_list
                add     hl,de
                ld      a,(hl)
                ret
drm_esc:        xor     a
                ret
drm_n:          db      0
drm_sel:        db      0
drm_i:          db      0

; F8 풀다운 메뉴 (F8 칸 위쪽): 이름 / 확장자 / 날짜. ↑↓ + RETURN (또는 N·E·D), ESC 취소 → A = 정렬 방식(0 이름, 1 확장자, 3 날짜), 취소면 0FFh
sort_menu:      ld      a,(sort_mode)
                ld      b,1
                cp      1
                jr      z,sx_i
                ld      b,2
                cp      3
                jr      z,sx_i
                ld      b,0
sx_i:           ld      a,b
                ld      (sx_sel),a
                ld      (sx_old),a
                xor     a
                ld      (sx_row),a
                ld      a,2
                ld      (sx_end),a              ; 처음에만 세 줄 모두 그림
                call    sx_dl
                jr      sx_key
; 고른 줄이 바뀌면 예전 줄과 새 줄만 다시 그림
sx_chg:         ld      a,(sx_old)
                call    sx_one
                ld      a,(sx_sel)
                ld      (sx_old),a
                call    sx_one
sx_key:         call    wait_key
                cp      1Bh
                jr      z,sx_cancel
                cp      0Dh
                jr      z,sx_pick
                cp      K_UP
                jr      nz,sx_k1
                ld      a,(sx_sel)
                or      a
                jr      z,sx_key
                dec     a
                ld      (sx_sel),a
                jr      sx_chg
sx_k1:          cp      K_DOWN
                jr      nz,sx_k2
                ld      a,(sx_sel)
                cp      2
                jr      nc,sx_key
                inc     a
                ld      (sx_sel),a
                jr      sx_chg
sx_k2:          and     0DFh
                ld      b,0
                cp      'N'
                jr      z,sx_set2
                inc     b
                cp      'E'
                jr      z,sx_set2
                inc     b
                cp      'D'
                jr      nz,sx_key
sx_set2:        ld      a,b
                ld      (sx_sel),a
                jr      sx_chg
sx_pick:        ld      a,(sx_sel)
                cp      2
                jr      nz,sx_ret
                ld      a,3
sx_ret:         ret
sx_cancel:      ld      a,0FFh
                ret
sx_one:         ld      (sx_row),a              ; A = 줄 하나만 그림
                ld      (sx_end),a
sx_dl:          ld      a,(sx_row)              ; 항목 i: 14 글자 (앞 한 칸은 고른 표시)
                ld      c,a
                add     a,a
                add     a,a
                add     a,a
                sub     c
                add     a,a                     ; ×14
                ld      e,a
                ld      d,0
                ld      hl,txt_smenu
                add     hl,de
                ld      de,qd_tmp
                ld      bc,14
                ldir
                ld      a,(sx_row)
                ld      b,a
                ld      a,(sx_sel)
                cp      b
                jr      nz,sx_nm
                ld      a,'>'
                ld      (qd_tmp),a
sx_nm:          ld      a,(sx_row)
                add     a,20
                call    row_addr
                ld      de,56
                add     hl,de
                ld      de,qd_tmp
                ld      b,14
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ld      a,(sx_row)
                inc     a
                ld      (sx_row),a
                ld      b,a
                ld      a,(sx_end)
                cp      b
                jr      nc,sx_dl
                ret
txt_smenu:      db      " Name         "
                db      " Extension    "
                db      " Date         "
sx_sel:         db      0
sx_row:         db      0
sx_end:         db      0
sx_old:         db      0
