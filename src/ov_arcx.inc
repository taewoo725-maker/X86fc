; 만든이: KimTaeWoo
; X86FC 압축 파일 풀기 (오버레이 13) - SPDX-License-Identifier: GPL-3.0-or-later
; ov_arc.inc(목록, 오버레이 6)에서 Enter·INS 로 건너옴.  끝나면 오버레이 6 으로 돌아감 (진입 1 = 반대 패널 다시 읽고 목록 그림, 2 = 목록만).
; 쪽 2 를 바꿔 끼운 채 실행되는 루틴(ar_ldd, ar_run)은 맨 앞(8000h 아래)에 둠.
                include "arcdef.inc"
; ---- (0.12.41) 쪽 2 를 바꾸는 동안 실행되는 루틴은 오버레이 맨 앞쪽 (7230h + 3536 = 8000h 아래) 에 둔다.  나머지는 8000h 를 넘어도 됨
; 디코더(오버레이 7~9)를 확보한 쪽에 읽어 둠 → Z = 성공
ar_ldd:         ld      a,(ar_seg)
                call    mp_putp2
                ld      hl,SELFPATH
                ld      de,CMDBUF
                call    copy_z
                xor     a
                ld      (de),a
                dec     de
                dec     de
                dec     de
                ld      a,'O'
                ld      (de),a
                inc     de
                ld      a,'V'
                ld      (de),a
                inc     de
                ld      a,'L'
                ld      (de),a
                ld      de,CMDBUF
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      nz,al_x
                ld      a,b
                ld      (ar_h2),a
                ld      a,(ar_fmt)              ; 디코더 칸 = 6 + 형식 (LZH 7, ZIP 8, ARJ 9)
                add     a,6
                ld      b,a
                ld      hl,0
                ld      de,OVSZ
al_m:           add     hl,de
                djnz    al_m
                ld      de,0
                ld      a,(ar_h2)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(ar_h2)
                ld      b,a
                ld      de,8000h
                ld      hl,OVSZ
                ld      c,F_READ
                call    BDOS
                ld      a,(ar_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                xor     a
al_x:           push    af
                ld      a,(ar_p2)
                call    mp_putp2
                pop     af
                or      a
                ret

; 매퍼 쪽을 끼워 디코더 실행 (8000h 위 오버레이 코드는 이 동안 가려지므로 이 루틴과 ar_ldd 는 오버레이 앞쪽(8000h 아래)에 둠)
ar_run:
                ld      a,(ar_seg)              ; --- 쪽을 끼우고 풀기
                call    mp_putp2
                ld      a,(AR_REC+R_MCH)
                ld      (8003h),a
                ld      a,(ar_h)
                ld      (8004h),a
                ld      a,(ar_ho)
                ld      (8005h),a
                ld      hl,(AR_REC+R_CSZ)
                ld      (8006h),hl
                ld      hl,(AR_REC+R_CSZ+2)
                ld      (8008h),hl
                ld      hl,(AR_REC+R_OSZ)
                ld      (800Ah),hl
                ld      hl,(AR_REC+R_OSZ+2)
                ld      (800Ch),hl
                ld      hl,(AR_REC+R_CRC)
                ld      (8011h),hl
                ld      hl,(AR_REC+R_CRC+2)
                ld      (8013h),hl
                ld      hl,ar_ptx
                ld      (800Fh),hl
                call    DEC_RUN
                ld      a,(800Eh)
                ld      (ar_res),a
                ld      a,(ar_p2)
                call    mp_putp2
                ret

; ---- 매퍼 보조 (편집기와 같음)
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
                add     hl,de
                ld      (mp_put),hl
                ld      de,3
                add     hl,de
                ld      (mp_get),hl
                or      a
                ret
mi_no:          scf
                ret
mp_alloc:       xor     a
                ld      b,0
                ld      hl,(mp_all)
                jp      (hl)
mp_free:        ld      b,0
                ld      hl,(mp_fre)
                jp      (hl)
mp_putp2:       ld      hl,(mp_put)
                jp      (hl)
mp_getp2:       ld      hl,(mp_get)
                jp      (hl)

; 메시지 팝업: HL = 글
ae_msg:         ld      de,txt_ppok
                call    popup_ask
ar_redraw:      jp      clear_screen

; RETURN / INS: 표시한 항목(없으면 커서 항목)을 반대편 패널 폴더로 풀기
ar_ext:         ld      hl,(ar_n)
                ld      a,h
                or      l
                jp      z,ar_toL2
                call    ar_cnt
                ld      a,h
                or      l
                jr      nz,ae_go
                ld      hl,(ar_cur)             ; 표시한 게 없으면 커서 항목만
                call    mk_addr
                or      (hl)
                ld      (hl),a
ae_go:          call    ar_dstf                 ; (0.12.55) 풀 폴더 고르기 풀다운(오버레이 15): 반대 패널 폴더에서 시작 (PROGBUF), 고르면 ar_ext2 로 돌아옴
                pop     hl
                ld      a,15
                ld      b,0
                jp      ov_go
ar_ext2:        ld      ix,(ar_pix)             ; 고른 폴더(PROGBUF)를 반대 패널의 폴더로 (풀고 나면 거기를 보여 줌)
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ex      de,hl
                ld      hl,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,0
                ld      (ar_xc),hl
                call    map_init
                jp      c,ae_nm
                call    mp_alloc
                jp      c,ae_nm
                ld      (ar_seg),a
                call    mp_getp2
                ld      (ar_p2),a
                call    ar_ldd
                jp      nz,ae_ld
                call    ar_dstf
                ld      hl,0
                ld      (ar_t),hl
                xor     a
                ld      (ar_ovr),a
ae_l:           ld      hl,(ar_t)
                ld      de,(ar_n)
                or      a
                sbc     hl,de
                jr      nc,ae_end
                ld      hl,(ar_t)
                call    mk_addr
                and     (hl)
                jr      z,ae_nx
                call    ar_one
                or      a
                jr      nz,ae_end
ae_nx:          ld      hl,(ar_t)
                inc     hl
                ld      (ar_t),hl
                jr      ae_l
ae_end:         ld      a,(ar_seg)
                call    mp_free
                ld      hl,AR_MARK
                ld      de,AR_MARK+1
                ld      bc,31
                ld      (hl),0
                ldir
                ld      de,errbuf               ; (0.12.55) 결과 안내: "Extracted N file(s) to 폴더"
                ld      hl,ar_tdone
                call    copy_z
                ld      hl,(ar_xc)
                call    put_dec16
                ld      hl,ar_tdone2
                call    copy_z
                ld      hl,PROGBUF
                ld      b,30
ae_pp:          ld      a,(hl)
                or      a
                jr      z,ae_pe
                ld      (de),a
                inc     hl
                inc     de
                djnz    ae_pp
ae_pe:          xor     a
                ld      (de),a
                ld      hl,errbuf
                call    ae_msg
                pop     hl                      ; 목록(오버레이 6)으로: 반대 패널을 다시 읽고 목록을 그림
                ld      a,6
                ld      b,1
                jp      ov_go
ae_ld:          ld      a,(ar_seg)
                call    mp_free
                ld      hl,ar_tnodec
                jr      ae_m1
ae_nm:          ld      hl,ar_tnomap
ae_m1:          call    ae_msg
                jp      ar_toL2

; ZIP: R_DOFF(지역 머리 위치) → 자료 위치
ar_zloc:        ld      hl,(AR_REC+R_DOFF)
                ld      (ar_pos),hl
                ld      hl,(AR_REC+R_DOFF+2)
                ld      (ar_pos+2),hl
                ld      bc,30
                call    ar_rdp
                ld      hl,(SECBUF+26)
                ld      de,(SECBUF+28)
                add     hl,de
                ld      de,30
                add     hl,de
                ld      de,0
                call    ar_add
                ld      hl,(ar_pos)
                ld      (AR_REC+R_DOFF),hl
                ld      hl,(ar_pos+2)
                ld      (AR_REC+R_DOFF+2),hl
                ret
; 반대 패널의 폴더 → PROGBUF (풀 곳), ar_pix = 반대 패널
ar_dstf:        call    cur_ix
                push    ix
                pop     hl
                ld      de,pstate
                or      a
                sbc     hl,de
                ld      ix,pstate+PSZ
                jr      z,ad_o
                ld      ix,pstate
ad_o:           ld      (ar_pix),ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                ret

; 항목 하나 풀기 (ar_t 번째) → A = 0 계속, 1 그만
ar_one:         ld      de,(ar_t)
                call    ar_rec
                ld      a,(AR_REC+R_MCH)
                cp      'd'
                jp      z,ao_ok                 ; 폴더 항목은 건너뜀 (파일 경로를 풀 때 만듦)
                call    ar_path                 ; CMDBUF = 풀 경로 (폴더 만들기 포함), AR_REC+88... 파일 이름 글은 ar_fn
                or      a
                jp      nz,ao_ok
                ld      de,CMDBUF               ; 이미 있나
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      nz,ao_new
                ld      c,F_CLOSE
                call    BDOS
                ld      a,(ar_ovr)
                or      a
                jr      nz,ao_new
                ld      de,errbuf               ; "Overwrite NAME?"
                ld      hl,ar_tovw
                call    copy_z
                ld      hl,ar_fn
                call    copy_z
                ld      a,'?'
                ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      de,txt_ppyna
                call    popup_ask
                cp      'N'
                jp      z,ao_ok
                cp      'A'
                jr      nz,ao_y
                ld      (ar_ovr),a
                jr      ao_new
ao_y:           cp      'Y'
                jp      nz,ao_stop              ; Esc 등 = 그만
ao_new:         ld      de,CMDBUF               ; 파일 만들기
                xor     a
                ld      b,0
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,ao_c
                call    explain_err
                jp      ao_stop
ao_c:           ld      a,b
                ld      (ar_ho),a
                ld      hl,ar_text              ; 진행 팝업 제목 "Extracting NAME"
                ld      de,ar_ptx
                call    copy_z
                ld      hl,ar_fn
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,(ar_fmt)              ; ZIP: 지역 머리(30 바이트)의 이름·덧붙임 길이만큼 더 가야 자료
                cp      2
                call    z,ar_zloc
                ld      a,(ar_h)                ; 압축 파일 읽을 위치로
                ld      b,a
                ld      hl,(AR_REC+R_DOFF)
                ld      de,(AR_REC+R_DOFF+2)
                xor     a
                ld      c,F_SEEK
                call    BDOS
                call    ar_run
                ld      a,(ar_res)
                or      a
                jr      nz,ao_err
                ld      hl,(AR_REC+R_DATE)      ; 날짜·시간 보존 (_HFTIME 쓰기: HL = 날짜, IX = 시간)
                ld      de,(AR_REC+R_TIME)
                push    ix
                push    de
                pop     ix
                ld      a,(ar_ho)
                ld      b,a
                ld      a,1
                ld      c,F_HFTIME
                call    BDOS
                pop     ix
                ld      a,(ar_ho)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      hl,(ar_xc)              ; (0.12.55) 푼 파일 수
                inc     hl
                ld      (ar_xc),hl
ao_ok:          xor     a
                ret
ao_err:         ld      a,(ar_ho)               ; 실패: 파일을 닫고 (CRC 오류는 지울지 물음)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                call    ar_redraw               ; 진행 팝업 지움
                ld      a,(ar_res)
                cp      1
                jr      nz,ao_e2
                ld      hl,ar_tcrc              ; "CRC error: delete the file?"
                ld      de,txt_ppyn
                call    popup_ask
                cp      'Y'
                jr      nz,ao_e3
                jr      ao_del
ao_e2:          cp      2
                jr      z,ao_del
                call    ao_dl                   ; 먼저 지움 (팝업이 CMDBUF 를 망가뜨림)
                ld      a,(ar_res)
                ld      hl,ar_tfail             ; 쓰기 오류·자료 오류·지원 안 함
                cp      5
                jr      nz,ao_e5
                ld      hl,ar_tunsup
                jr      ao_e4
ao_e5:          cp      6                       ; 창이 모자람 (여분 매퍼 쪽 부족)
                jr      nz,ao_e4
                ld      hl,ar_tnomap
ao_e4:          ld      de,errbuf
                call    copy_z
                ld      a,' '
                ld      (de),a
                inc     de
                ld      a,(ar_res)
                add     a,'0'
                ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                ld      hl,errbuf
                call    ae_msg
                jr      ao_e3
ao_del:         call    ao_dl
ao_e3:          ld      a,(ar_res)
                cp      2
                jr      z,ao_stop
                jr      ao_ok
ao_stop:        ld      a,1
                ret
ao_dl:          ld      de,CMDBUF
                ld      c,F_DELETE
                jp      BDOS

; ---- 풀 경로 만들기: CMDBUF = PROGBUF(풀 폴더) + 이름의 폴더 부분(만들며) + 8.3 로 고친 파일 이름.  ar_fn = 파일 이름 글. A = 0 성공
ar_path:        ld      hl,PROGBUF
                ld      de,CMDBUF
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,ap_s
                ld      a,'\'
                ld      (de),a
                inc     de
ap_s:           ld      hl,AR_REC+R_NAME
ap_l:           push    de                      ; 이 성분의 시작
                call    ar_comp                 ; HL = 구분자/끝, DE = 성분 끝, A = 구분자 (0 = 끝)
                pop     bc                      ; BC = 성분 시작
                or      a
                jr      z,ap_end
                push    af
                ld      a,e                     ; 빈 성분이면 건너뜀
                cp      c
                jr      nz,ap_d
                ld      a,d
                cp      b
                jr      nz,ap_d
                pop     af
                inc     hl
                jr      ap_l
ap_d:           pop     af
                inc     hl
                push    hl
                xor     a
                ld      (de),a
                push    de
                ld      de,CMDBUF               ; 폴더 만들기 (이미 있으면 오류 — 무시)
                xor     a
                ld      b,10h
                ld      c,F_CREATE
                call    BDOS
                pop     de
                ld      a,'\'
                ld      (de),a
                inc     de
                pop     hl
                jr      ap_l
ap_end:         push    de                      ; 파일 이름 성분 = BC ~ DE, 비었으면 실패
                ld      h,d
                ld      l,e
                or      a
                sbc     hl,bc
                pop     de
                jr      z,ap_bad
                xor     a
                ld      (de),a
                ld      h,b
                ld      l,c
                ld      de,ar_fn
                call    copy_z
                xor     a
                ret
ap_bad:         ld      a,1
                ret
; HL = 원본 이름, DE = 쓸 곳 → 한 성분을 8.3 안전 글로 DE 에 쓰고 DE 전진, HL = 구분자/끝, A = 구분자 (끝 = 0)
ar_comp:        xor     a
                ld      (acq_m),a
                ld      (acq_n),a
ac_c:           ld      a,(hl)
                or      a
                ret     z
                cp      '/'
                jr      z,ac_r
                cp      '\'
                jr      z,ac_r
                cp      0FFh
                jr      z,ac_r
                cp      '.'
                jr      nz,ac_nd
                ld      a,(acq_m)                ; 첫 점이고 앞에 글이 있으면 확장자로
                or      a
                jr      nz,ac_sk
                ld      a,(acq_n)
                or      a
                jr      z,ac_sk
                ld      a,1
                ld      (acq_m),a
                xor     a
                ld      (acq_n),a
                ld      a,'.'
                ld      (de),a
                inc     de
                jr      ac_sk
ac_nd:          ld      b,a                     ; 일반 글자
                ld      a,(acq_m)
                or      a
                ld      a,8
                jr      z,ac_lm
                ld      a,3
ac_lm:          ld      c,a
                ld      a,(acq_n)
                cp      c
                jr      nc,ac_sk
                inc     a
                ld      (acq_n),a
                ld      a,b
                call    ac_fix
                ld      (de),a
                inc     de
ac_sk:          inc     hl
                jr      ac_c
ac_r:           ret
; A = 글자 → 대문자·허용 글자 (그 밖 = _)
ac_fix:         cp      'a'
                jr      c,af_u
                cp      'z'+1
                jr      nc,af_u
                and     0DFh
af_u:           cp      'A'
                jr      c,af_n
                cp      'Z'+1
                jr      c,af_ok
af_n:           cp      '0'
                jr      c,af_s
                cp      '9'+1
                jr      c,af_ok
af_s:           push    hl
                push    bc
                ld      hl,txt_sym83
                ld      bc,16
                cpir
                pop     bc
                pop     hl
                ret     z
                ld      a,'_'
af_ok:          ret

ar_tdone:       db      "Extracted ",0
ar_tdone2:      db      " file(s) to ",0
ar_tnomap:      db      "Need more mapper pages",0
ar_tnodec:      db      "Cannot load decoder",0
ar_tovw:        db      "Overwrite ",0
ar_text:        db      "Extracting ",0
ar_tcrc:        db      "CRC error. Delete?",0
ar_tfail:       db      "Extract failed",0
ar_tunsup:      db      "Unsupported method",0

ar_toL2:        pop     hl                      ; 목록(오버레이 6)으로: 목록만 다시 그림
                ld      a,6
                ld      b,2
                jp      ov_go
                include "arccom.inc"
