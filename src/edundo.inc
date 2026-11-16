; 만든이: KimTaeWoo
; X86FC 편집기 되돌리기 기록 (오버레이 1·10 이 같이 씀) - SPDX-License-Identifier: GPL-3.0-or-later
; 기록 하나 = [지운 글 dl 바이트][자리 2][지운 길이 dl 2][끼운 길이 sl 2] 를 LOG 에 쌓음 (맨 위 = ud_top).  글을 바꾸기 *전에* 부름
; (글은 쪽 2 에 끼워진 상태).  글 한 바이트 입력·지우기마다 하나씩 쌓이고, 가득 차면 전부 버리고 새로 시작.
; 입력: sp_pos = 자리, sp_dl = 지울 길이 (그 자리의 글을 저장), sp_sl = 끼울 길이.  모든 레지스터를 바꿈.
ud_push:        ld      a,(ud_off)
                or      a
                ret     nz
                ld      hl,(ud_top)
                ld      a,h
                or      l
                jr      nz,ud_a
                ld      hl,LOG
ud_a:           call    ud_fit                  ; C = 맞음
                jr      c,ud_w
                ld      hl,LOG                  ; 가득: 처음부터 다시
                call    ud_fit
                jr      c,ud_w
                ld      hl,LOG                  ; 이 기록 하나가 너무 큼: 기록하지 않고 비움
                ld      (ud_top),hl
                ret
ud_w:           ld      bc,(sp_dl)
                ld      a,b
                or      c
                jr      z,ud_t
                ex      de,hl                   ; DE = 쓸 자리
                ld      hl,(sp_pos)
                ldir
                ex      de,hl
ud_t:           ld      de,(sp_pos)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                inc     hl
                ld      de,(sp_dl)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                inc     hl
                ld      de,(sp_sl)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                inc     hl
                ld      (ud_top),hl
                ret
; HL = 쓸 자리 → C = (자리 + dl + 6) 이 LOG 끝(LOG+LOGSZ) 안.  HL 보존
ud_fit:         push    hl
                ld      de,(sp_dl)
                add     hl,de
                ld      de,6
                add     hl,de
                ld      de,LOG+LOGSZ+1
                or      a
                sbc     hl,de
                pop     hl
                ret
