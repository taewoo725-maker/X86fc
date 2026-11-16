; 만든이: KimTaeWoo
; X86FC 오버레이 적재기 - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 덩치 큰 모듈(찾기·트리 등)은 X86FC.OVL 파일에 따로 두고, 쓸 때만 창(OVBASE, OVSZ 바이트)에 읽어 와 실행한다.
;   X86FC.OVL = 크기 OVSZ 의 칸(오버레이 번호 순)을 이어 붙인 파일.  오버레이 머리는 "JP 진입점" 표(3 바이트씩).
;   본체의 진입 껍데기:  라벨: ld a,<오버레이 번호> / ld b,<진입 번호> / jp ov_go
;   오버레이 안에서 본체 함수는 그 주소를 그대로 부름(빌드가 만든 ovsym.inc).  오버레이끼리는 부르지 않음.
; ----------------------------------------------------------------------
ml_umenu:       ld      a,2                     ; 사용자 메뉴 = 오버레이 2 (진입 0)
                ld      b,0
                jp      ov_go
ov_cur:         db      0FFh                   ; 창에 올라와 있는 오버레이 번호 (FFh = 없음)
ov_id:          db      0
ov_rc:          db      0
; A = 오버레이 번호, B = 진입 번호
ov_go:          push    bc
                ld      hl,ov_cur
                cp      (hl)
                jr      z,og_run
                ld      (ov_id),a
                call    ov_load
                jr      z,og_ok
                pop     bc
                ld      hl,txt_ovnf
                ld      (msg_p),hl
                ret
og_ok:          ld      a,(ov_id)
                ld      (ov_cur),a
og_run:         pop     bc
                ld      c,a                     ; C = 칸 번호
                ld      hl,ov_ret               ; (0.12.21) 모든 칸이 끝나면 ov_ret(h_on): 얇은 선 배치 복귀 + 해시 비움
                push    hl
                xor     a
                ld      (s7_th),a               ; 오버레이는 균일 격자
                inc     a
                ld      (s7_ovb),a
                ld      a,c
                cp      2
                jr      nc,og_pop
                cp      1                       ; (0.12.17) 편집기(1번 칸)는 7H 격자를 유지
                jr      z,og_go
                xor     a                       ; 0번 칸: 8점 줄 (7H 에서도)
                ld      (s7_h),a
                jr      og_go
; (0.12.32) 팝업 목록 칸(사용자 메뉴 2, 열기 목록 3, 이동·드라이브·정렬 5)은 얇은 선 배치를 유지: 안에서 draw_all 을 부르는데 균일 격자로
;  그리면 줄 표(SCREEN 7 은 28줄)가 화면 밖(y 212~)에 써서 DOS 화면 맡김(VRAM D400h~)을 깨뜨림.  도움말(3번 칸 2번 진입)·보기(4)는 균일 격자
og_pop:         cp      4
                jr      z,og_go
                cp      3
                jr      nz,og_pp
                ld      a,b
                cp      2
                jr      z,og_go
og_pp:          ld      a,(s7_thm)
                ld      (s7_th),a
                xor     a
                ld      (s7_ovb),a
og_go:          ld      l,b
                ld      h,0
                ld      e,l
                ld      d,h
                add     hl,hl
                add     hl,de                   ; 3 x 진입 번호
                ld      de,OVBASE
                add     hl,de
                jp      (hl)
; X86FC.OVL 에서 ov_id 번 칸을 OVBASE 로 읽음 → Z = 성공
ov_load:        ld      hl,SELFPATH             ; 경로 = X86FC.COM 의 끝 세 글자를 OVL 로
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
                ret     nz
                ld      a,b
                ld      (h_src),a
                ld      a,(ov_id)
                ld      b,a
                ld      hl,0
                or      a
                jr      z,ol_s
                ld      de,OVSZ
ol_m:           add     hl,de
                djnz    ol_m
ol_s:           ld      de,0
                ld      a,(h_src)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(h_src)
                ld      b,a
                ld      de,OVBASE
                ld      hl,OVSZ
                ld      c,F_READ
                call    BDOS
                ld      (ov_rc),a
                ld      a,(h_src)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      a,(ov_rc)
                cp      E_EOF
                ret     z
                or      a
                ret
txt_ovnf:       db      "X86FC.OVL not found or unreadable",0
txt_notfile:    db      "Not a file",0                  ; (보기·편집기·열기 목록이 같이 씀 → 상주)
; (열기 목록 오버레이와 본체가 같이 쓰는 것)
vl_n:           db      0
txt_vw:         db      "view"
