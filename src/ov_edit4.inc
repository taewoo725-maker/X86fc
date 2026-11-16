; 만든이: KimTaeWoo
; X86FC 편집기 EUC-KR 저장 (오버레이 12) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; EUC-KR 글을 UTF-8 로 바꿔 열었으면(ed_euc = 1) 저장(Ctrl+S = 명령 20, 닫을 때 저장 = 명령 21)은 여기서 한다:
;   표(edeuc.inc)를 글 쪽 끝에 읽고, 글의 한글 음절을 KS X 1001 두 바이트로 되돌려 파일에 씀.  표에 없는 글자·한글 밖의 글자가 있거나
;   글이 너무 길어(13960바이트 초과) 표 자리가 없으면 알리고 UTF-8 로 저장할지 물음 (예 = UTF-8 로 저장하고 이후엔 UTF-8 글).
; 표 자리가 되돌리기 기록 자리를 덮으므로 기록은 비움.  쪽 2 의 글이 끼워진 동안 코드·자료는 8000h 아래여야 함 (3536 바이트 이하).
; ----------------------------------------------------------------------
                include "edvars.inc"
                include "edeuc.inc"
EDBUF_          equ     EDBUF

mp_putp2:       ld      hl,(mp_put)
                jp      (hl)
e2_map:         ld      a,(ed_seg)
                jr      mp_putp2
e2_unmap:       ld      a,(ed_old)
                jr      mp_putp2
c_back:         pop     hl                      ; ov_ret 버리고 편집기로
                xor     a
                ld      b,a
                inc     a
                jp      ov_go

euc_save:       call    e2_map
                ld      hl,LOG                  ; 표가 기록 자리를 덮음
                ld      (ud_top),hl
                ld      hl,(ed_end)
                ld      de,TBLA
                or      a
                sbc     hl,de
                jr      nc,es_no                ; 글이 길어 표 자리가 없음
                call    eu_load
                jr      nz,es_no
                xor     a
                ld      (eu_mode),a
                ld      hl,0
                ld      (ed_cnt),hl
                call    eu_run                  ; 못 바꾸는 글자 세기
                ld      hl,(ed_cnt)
                ld      a,h
                or      l
                jr      nz,es_no
                ld      a,1                     ; EUC-KR 로 씀
                ld      (eu_mode),a
                call    es_create
                jp      nz,es_fail
                xor     a
                ld      (eu_c),a
                call    eu_run
                call    er_flush
                call    es_close
                ld      a,(eu_c)
                or      a
                jr      nz,es_fail
                jr      es_ok
es_no:          call    e2_unmap                ; EUC-KR 로 못 저장 → UTF-8 로 저장할지 물음
                ld      hl,txt_euu
                call    popup_yn
                jp      nz,c_back
                call    e2_map
                call    es_create
                jr      nz,es_fail
                ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      a,h
                or      l
                jr      z,es_u
                ld      a,(ed_h)
                ld      b,a
                ld      de,EDBUF
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      nz,es_f2
es_u:           call    es_close
                xor     a
                ld      (ed_euc),a
es_ok:          xor     a
                ld      (ed_mod),a
                ld      hl,txt_saved
                ld      (msg_p),hl
                call    e2_unmap
                ld      a,(ed_cmd)
                cp      21
                jp      nz,c_back
                call    hk_off                  ; 저장하고 닫기
                xor     a
                ld      (hk_st),a
                ld      a,(ed_seg)
                ld      b,0
                ld      hl,(mp_fre)
                call    es_jp
                ld      a,(clip_seg)
                or      a
                jr      z,ef_n
                ld      b,0
                ld      hl,(mp_fre)
                call    es_jp
ef_n:
                xor     a
                ld      (CSRSW),a
                ret
es_jp:          jp      (hl)
es_f2:          push    af
                call    es_close
                pop     af
es_fail:        call    e2_unmap
                ld      hl,txt_esf
                ld      de,txt_ppok
                call    popup_ask
                jp      c_back
; 파일 다시 만들기(속성 유지) → NZ = 실패, 핸들 ed_h.  닫기
es_create:      ld      de,srcpath
                ld      a,(ed_attr)
                ld      b,a
                xor     a
                ld      c,F_CREATE
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (ed_h),a
                xor     a
                ret
es_close:       ld      a,(ed_h)
                ld      b,a
                ld      c,F_CLOSE
                jp      BDOS
; ---- 글을 훑으며 (eu_mode 0 = 못 바꾸는 글자 수를 ed_cnt 에 셈, 1 = EUC-KR 바이트를 BUFC 에 모아 파일에 씀)
eu_run:         ld      hl,EDBUF
                xor     a
                ld      (eu_o),a
er_l:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                ret     nc
                ld      a,(hl)
                inc     hl
                cp      80h
                jr      nc,er_h
                call    er_put
                jr      er_l
er_h:           cp      0C0h
                jr      c,er_bad                ; 혼자 온 이어지는 바이트
                cp      0E0h
                jr      c,er_s1
                cp      0F0h
                jr      nc,er_s3
                cp      0EAh
                jr      c,er_s2
                cp      0EEh
                jr      nc,er_s2
                jr      er_hg                   ; EA~ED = 한글 음절 후보
er_s1:          ld      b,1
                jr      er_sk
er_s2:          ld      b,2
                jr      er_sk
er_s3:          ld      b,3
er_sk:          ld      de,(ed_end)             ; 이어지는 바이트 B 개를 건너뜀
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,er_bad
                ld      a,(hl)
                and     0C0h
                cp      80h
                jr      nz,er_bad
                inc     hl
                djnz    er_sk
er_bad:         ld      a,(eu_mode)             ; 못 바꾸는 글자: 세기 방식이면 세고, 쓰기 방식이면 '?'
                or      a
                jr      nz,er_q
                push    hl
                ld      hl,(ed_cnt)
                inc     hl
                ld      (ed_cnt),hl
                pop     hl
                jr      er_l
er_q:           ld      a,'?'
                call    er_put
                jr      er_l
; 한글 음절 후보: A = 첫 바이트 (EA~ED), HL = 둘째 바이트 자리.  코드 = (첫&0F)<<12 | (둘째&3F)<<6 | (셋째&3F)
er_hg:          and     0Fh
                add     a,a
                add     a,a
                add     a,a
                add     a,a
                ld      d,a                     ; D = (첫&0F) << 4
                ld      bc,(ed_end)
                push    hl
                inc     hl
                or      a
                sbc     hl,bc
                pop     hl
                jr      nc,er_bad               ; 바이트가 모자람
                ld      a,(hl)
                ld      c,a
                and     0C0h
                cp      80h
                jr      nz,er_bad
                inc     hl
                ld      a,(hl)
                ld      b,a
                and     0C0h
                cp      80h
                jr      nz,er_bad
                inc     hl
                ld      (eu_src),hl             ; 다음 글자 자리
                ld      a,c
                and     3Fh
                ld      c,a
                srl     a
                srl     a
                or      d
                ld      d,a                     ; D = 코드 윗바이트
                ld      a,c
                and     3
                rrca
                rrca
                ld      e,a
                ld      a,b
                and     3Fh
                or      e
                ld      e,a                     ; E = 코드 아랫바이트
                ld      a,d
                cp      0ACh
                jr      c,er_bd2
                cp      0D8h
                jr      nc,er_bd2
                sub     0ACh
                ld      h,a
                ld      l,e                     ; HL = idx
                call    eu_find
                jr      c,er_bd2
                ld      b,0                     ; n → 첫 = B0h + n / 94, 둘째 = A1h + n % 94
                ld      de,94
er_dv:          or      a
                sbc     hl,de
                jr      c,er_dd
                inc     b
                jr      er_dv
er_dd:          add     hl,de
                ld      a,l
                add     a,0A1h
                ld      (ed_t1),a
                ld      a,b
                add     a,0B0h
                call    er_put
                ld      a,(ed_t1)
                call    er_put
                ld      hl,(eu_src)
                jp      er_l
er_bd2:         ld      hl,(eu_src)
                jp      er_bad
; A = 바이트 → (쓰기 방식이면) BUFC 에 모음, 가득이면 파일에 씀.  HL 보존
er_put:         ld      c,a
                ld      a,(eu_mode)
                or      a
                ret     z
                push    hl
                ld      a,(eu_o)
                ld      e,a
                ld      d,0
                ld      hl,BUFC
                add     hl,de
                ld      (hl),c
                ld      a,(eu_o)
                inc     a
                ld      (eu_o),a
                cp      250
                call    nc,er_flush
                pop     hl
                ret
er_flush:       ld      a,(eu_o)
                or      a
                ret     z
                ld      l,a
                ld      h,0
                ld      a,(ed_h)
                ld      b,a
                ld      de,BUFC
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      z,ef_k
                ld      (eu_c),a
ef_k:           xor     a
                ld      (eu_o),a
                ret
txt_euu:        db      "Cannot save as EUC-KR. Save as UTF-8?",0
txt_esf:        db      "Save failed",0
txt_saved:      db      "Saved",0
