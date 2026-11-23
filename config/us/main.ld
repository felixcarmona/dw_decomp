#include <dw/memmap.h>

ENTRY(__start)

SECTIONS
{
    __romPos = 0;
    _gp = main_SDATA_START + 0x8000;
    header_ROM_START = __romPos;
    .header EXE_HEADER_ADDR : AT(header_ROM_START)
    {
        KEEP(BUILD_DIR/asm/main/data/header.data.s.o(.data.*));
        . = ALIGN(., 4);
    }
    __romPos += SIZEOF(.header);
    __romPos = ALIGN(__romPos, 4);
    . = ALIGN(., 4);

    main_ROM_START = __romPos;
    .main : AT(main_ROM_START)
    {
        main_TEXT_START = .;
        BUILD_DIR/asm/main/psyq/libapi.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libc.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libetc.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libgpu.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libgs.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libgte.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/model.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/world_object.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/math.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/graphics.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/drop_shadow.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/_psstart.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fade.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/aabb.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libcd.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_collision.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/particle.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/anim.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/ui.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libspu.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libsnd.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/inventory.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_object.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evolution.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/line.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/vecmath.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/rng.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fish.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils2.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.text.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.text.*);
        BUILD_DIR/src/main/mod_qol.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tournament.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evl.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/btl.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/kar.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe_table.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound_async.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libds.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/butterfly.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libmcrd.s.o(.text .text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/bubble.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_tick.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/new_game.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/buff_model.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sjis.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_textbox.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_value.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/shop/script_menu.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.text.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.text.*);
        . = ALIGN(., 16);
        main_TEXT_END = .;
        main_TEXT_SIZE = ABSOLUTE(main_TEXT_END - main_TEXT_START);

        main_RODATA_START = .;

        __static_init = .;
        LONG(0x0)
        _gp_addr = .;
        LONG(_gp)
        _sbss_size = .;
        LONG(main_SBSS_SIZE)
        _bss_size = .;
        LONG(main_BSS_SIZE)
        _sdata_size = .;
        LONG(main_SDATA_SIZE)
        _data_size = .;
        LONG(main_DATA_SIZE)
        _rdata_size = .;
        LONG(main_RODATA_SIZE)
        _fsbss = .;
        LONG(main_SBSS_START)
        _fbss = .;
        LONG(main_BSS_START)
        _fsdata = .;
        LONG(main_SDATA_START)
        _fdata = .;
        LONG(main_DATA_START)
        _frdata = .;
        LONG(main_RODATA_START)
        _start_etbl = .;
        LONG(main_TEXT_START)
        _nbfunctions = .;
        LONG(main_TEXT_START)
        _ftext = .;
        LONG(main_TEXT_START)
        _stack_addr = .;
        LONG(STACK_ADDR)
        _nbtvectors = .;
        LONG(0x0)
        _stack_size = .;
        LONG(STACK_SIZE_KB)
        _heap_size = .;
        LONG(0x400)
        _etext = .;
        LONG(main_TEXT_END)
        _edata = .;
        LONG(main_SDATA_END)
        _end = .;
        LONG(main_BSS_END)
        _fsdata_ROM = .;
        LONG(main_SDATA_START)
        _fdata_ROM = .;
        LONG(main_DATA_START)
        _sdata_ROM_size = .;
        LONG(main_SDATA_SIZE)
        _data_ROM_size = .;
        LONG(main_DATA_SIZE)
        _scratchpad_size = .;
        LONG(0x0)

        _overlay_section_addresses = .;
        LONG(main_TEXT_START)
        LONG(btl_HEADER)
        LONG(std_HEADER)
        LONG(fish_HEADER)
        LONG(evl_HEADER)
        LONG(kar_HEADER)
        LONG(vs_HEADER)
        LONG(mov_HEADER)
        LONG(doo2_HEADER)
        LONG(dooa_HEADER)
        LONG(trn_HEADER)
        LONG(dget_HEADER)
        LONG(trn2_HEADER)
        LONG(murd_HEADER)
        LONG(endi_HEADER)
        LONG(eab_HEADER)

        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libc.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libetc.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libgpu.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libgs.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libgte.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/model.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/world_object.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/math.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_table_us.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/graphics.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/drop_shadow.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/_psstart.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fade.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/aabb.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libcd.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_collision.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/particle.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/anim.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/ui.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libspu.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libsnd.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/inventory.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_object.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evolution.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/line.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/vecmath.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/rng.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fish.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils2.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.rodata .rodata.*);
        BUILD_DIR/src/main/mod_qol.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tournament.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evl.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/btl.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/kar.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe_table.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound_async.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libds.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/butterfly.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libmcrd.rodata.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/bubble.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_tick.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/new_game.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/buff_model.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sjis.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_textbox.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_value.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/shop/script_menu.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.rodata.* .rodata);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libapi.s.o(.rodata .rodata.*);
        . = ALIGN(., 4);
        main_RODATA_END = .;
        main_RODATA_SIZE = ABSOLUTE(main_RODATA_END - main_RODATA_START);

        main_DATA_START = .;
        BUILD_DIR/asm/main/data/psyq/libapi.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libc.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libetc.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libgpu.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libgs.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libgte.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/model.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/world_object.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/math.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_table_us.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/graphics.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/drop_shadow.c.o(.data.*);
        . = ALIGN(., 4);
        KEEP(BUILD_DIR/src/main/_psstart.c.o(.data.*));
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fade.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/aabb.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/toilet_data.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/door_mapdata.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libcd.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_collision.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/particle.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/anim.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/ui.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libspu.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libsnd.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/inventory.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_object.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evolution.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/line.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/vecmath.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/rng.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fish.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils2.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.data.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.data.*);
        BUILD_DIR/src/main/mod_qol.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tournament.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evl.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/btl.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/kar.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe_table.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound_async.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/psyq/libds.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/butterfly.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/asm/main/data/psyq/libmcrd.data.s.o(.data .data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/bubble.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_ui.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_tick.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/new_game.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/buff_model.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sjis.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_combat.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_textbox.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_value.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/shop/script_menu.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sjis.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_combat.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_combat.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.data.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.data.*);
        . = ALIGN(., 4);
        main_DATA_END = .;
        main_DATA_SIZE = ABSOLUTE(main_DATA_END - main_DATA_START);

        main_SDATA_START = .;
        BUILD_DIR/src/main/model.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_table_us.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/ui.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/inventory.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_object.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/line.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/vecmath.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/rng.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils2.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.sdata.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.sdata.*);
        BUILD_DIR/src/main/mod_qol.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe_table.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound_async.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/bubble.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_ui.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_combat.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_value.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/shop/script_menu.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_setup.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_main.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/command_shout.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_hud.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_effect.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_setup.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_main.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_hud.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_effect.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/fish/fish.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/evl/evl.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/kar/kar.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_scene.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_camera.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_main.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_hud.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_effect.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/doo2/doo2.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/dooa/dooa.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn/trn_hud.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn/trn_slots.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn2/trn2_hud.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn2/trn2_slots.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/murd/murd.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/endi/endi.c.o(.sdata.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/eab/eab.c.o(.sdata.*);
        . = ALIGN(., 4);
        main_SDATA_END = .;
        main_SDATA_SIZE = ABSOLUTE(main_SDATA_END - main_SDATA_START);

        main_SBSS_START = .;
        . = ALIGN(., 0x800);
    }
    __romPos += SIZEOF(.main);
    __romPos = ALIGN(__romPos, 4);
    main_ROM_END = __romPos;
    main_ROM_SIZE = ABSOLUTE(main_ROM_END - main_ROM_START);
    . = ALIGN(., 4);

    sbss_ROM_START = __romPos;
    .sbss main_SBSS_START : AT(sbss_ROM_START)
    {
        BUILD_DIR/src/main/partner_impl.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fade.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/inventory.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner.c.o(.sbss.*);
        . = ALIGN(., 4);
        KEEP(BUILD_DIR/src/main/map_object.c.o(.sbss.*));
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evolution.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/rng.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fish.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/utils2.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.sbss.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.sbss.*);
        BUILD_DIR/src/main/mod_qol.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/world_object.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evl.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/btl.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/kar.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound_async.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_ui.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_tick.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/new_game.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/buff_model.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_combat.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_textbox.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_setup.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/command_menu.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_hud.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/btl/battle_effect.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_setup.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_main.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_hud.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/std/std_effect.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/fish/fish.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/evl/evl.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/kar/kar.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_scene.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_camera.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_hud.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_effect.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/mov/mov.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/doo2/doo2.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/dooa/dooa.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn/trn_hp.c.o(.sbss.*);
        BUILD_DIR/src/trn/trn_mp.c.o(.sbss.*);
        BUILD_DIR/src/trn/trn_off.c.o(.sbss.*);
        BUILD_DIR/src/trn/trn_def.c.o(.sbss.*);
        BUILD_DIR/src/trn/trn_hud.c.o(.sbss.*);
        BUILD_DIR/src/trn/trn_reward.c.o(.sbss.*); /* MOD (training block) */
        . = ALIGN(., 4);
        BUILD_DIR/src/dget/dget.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/trn2/trn2_hp_map99.c.o(.sbss.*);
        BUILD_DIR/src/trn2/trn2_def_map99.c.o(.sbss.*);
        BUILD_DIR/src/trn2/trn2_off.c.o(.sbss.*);
        BUILD_DIR/src/trn2/trn2_reward.c.o(.sbss.*);
        BUILD_DIR/src/trn2/trn2_hud.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/murd/murd.c.o(.sbss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/eab/eab.c.o(.sbss.*);
        . = ALIGN(., 4);
        main_SBSS_END = .;
        main_SBSS_SIZE = ABSOLUTE(main_SBSS_END - main_SBSS_START);
        main_BSS_START = .;
    }
    __romPos += SIZEOF(.sbss);
    __romPos = ALIGN(__romPos, 4);
    . = ALIGN(., 4);

    bss_ROM_START = __romPos;
    .bss main_BSS_START : AT(bss_ROM_START)
    {
        BUILD_DIR/generated/libapi.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libetc.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libgpu.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libgs.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libgte.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/model.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/world_object.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/graphics.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner_impl.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_lab.c.o(.bss.*);
        . = ALIGN(., 4);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_species.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_screen.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_options.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/mod_feed.c.o(.bss.*);
        BUILD_DIR/src/main/mod_qol.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tfs_stream.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_trim.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/npc_anim_keep.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/game_menu.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/tamer.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libcd.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/overworld.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/particle.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/ui.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/item.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/sound.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libspu.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libsnd.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/clock.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/partner.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/entity_text.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/map_object.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/file_queue.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/fish.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/evl.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/btl.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/efe_table.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libds.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/butterfly.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/generated/libmcrd.bss.s.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/bubble.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/battle_ui.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/new_game.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/buff_model.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_intro.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/vs/vs_select.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_common.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/script_interp.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/font.c.o(.bss.*);
        . = ALIGN(., 4);
        BUILD_DIR/src/main/main_menu.c.o(.bss.*);
        . = ALIGN(., 4);
        main_BSS_END = .;
        main_BSS_SIZE = ABSOLUTE(main_BSS_END - main_BSS_START);
        main_MEMFILL_SIZE = ABSOLUTE(main_BSS_END - main_SBSS_START);
        main_MEMFILL_ALIGNED_SIZE = ABSOLUTE(((main_MEMFILL_SIZE + 0X7FF) / 0x800) * 0x800);
    }
    __romPos += SIZEOF(.bss);
    __romPos = ALIGN(__romPos, 4);
    . = ALIGN(., 4);

    /DISCARD/ :
    {
        *(*);
    }
}

/*
 * Memory map checks (include/dw/memmap.h, docs/mods/ids.md § 9.5): a link
 * that would overlap the fixed areas fails here instead of in the game.
 */
#define OVL_END(s) (ADDR(s) + SIZEOF(s))
ASSERT(OVL_END(.MOV) <= LOAD_AREA_END, "MOV overlay overflows the load area")
ASSERT(OVL_END(.BTL) <= LOAD_AREA_END, "BTL overlay overflows the load area")
ASSERT(OVL_END(.STD) <= LOAD_AREA_END, "STD overlay overflows the load area")
ASSERT(OVL_END(.VS) <= LOAD_AREA_END, "VS overlay overflows the load area")
ASSERT(OVL_END(.KAR) <= LOAD_AREA_END, "KAR overlay overflows the load area")
ASSERT(OVL_END(.EVL) <= LOAD_AREA_END, "EVL overlay overflows the load area")
/* MOD: readFile loads whole sectors; FISH/DOO2 start at OVL_FISH_BASE (the shard scratch grew the overlay) */
ASSERT(ADDR(.EVL) + ((SIZEOF(.EVL) + 0x7FF) & ~0x7FF) <= OVL_FISH_BASE, "EVL overlay (in whole sectors) overlaps FISH")
ASSERT(OVL_END(.ENDI) <= LOAD_AREA_END, "ENDI overlay overflows the load area")
ASSERT(OVL_END(.EAB) <= LOAD_AREA_END, "EAB overlay overflows the load area")
ASSERT(OVL_END(.FISH) <= LOAD_AREA_END, "FISH overlay overflows the load area")
ASSERT(OVL_END(.DOO2) <= LOAD_AREA_END, "DOO2 overlay overflows the load area")
ASSERT(OVL_END(.MURD) <= OVL_OT_0_ADDR, "MURD overlay overlaps its ordering table")
ASSERT(OVL_END(.DOOA) <= OVL_OT_0_ADDR, "DOOA overlay overlaps its ordering tables")
ASSERT(OVL_END(.DGET) <= LOAD_AREA_END, "DGET overlay overflows the load area")
ASSERT(OVL_END(.TRN) <= OVL_TRN_MAX_END, "TRN overlay is larger than the original's slot")
ASSERT(OVL_END(.TRN2) <= OVL_TRN_MAX_END, "TRN2 overlay is larger than the original's slot")
/*
 * MOD (code out of the executable, docs/mods/memory.md): BTL and VS carry code the executable used to hold (battle_ui.c;
 * vs_combat.c, vs_intro.c), appended at their ends. A battle-mode overlay may reach as high as STD, the tournament
 * overlay that runs the same battle engine (EFE heap, effects, sound) and the original's largest. MOV carries libpress
 * and libcd's streaming: it must stay below TRN's slot (TEXTURE_BUFFER, the frame OTs of main.c).
 */
ASSERT(OVL_END(.BTL) <= OVL_END(.STD), "BTL overlay ends above STD's end, the highest a battle overlay is known to reach")
ASSERT(OVL_END(.VS) <= OVL_END(.STD), "VS overlay ends above STD's end, the highest a battle overlay is known to reach")
ASSERT(OVL_END(.MOV) <= OVL_TRN_BASE, "MOV overlay overlaps TRN's slot (TEXTURE_BUFFER)")
/* MOD (menu overlay): the save machine and the game-clear save run in the world, the latter with ENDI loading at
 * OVL_EVL_BASE (load_ending); the world's scratch with SAVE_EXT is below OVL_BTL_BASE */
ASSERT(OVL_END(.MENU) <= OVL_EVL_BASE, "MENU overlay overlaps EVL/ENDI's slot")
ASSERT(TFS_RING_ADDR + TFS_RING_SLOTS * TFS_SLOT_SIZE == TFS_RING_END,
       "TFS ring size does not match TFS_RING_END")
ASSERT(TFS_RING_END <= OVL_KAR_BASE, "TFS ring overlaps KAR, which coexists with the curling map")
ASSERT(SAVE_EXT_ADDR >= WORLD_SCRATCH_START && SAVE_EXT_ADDR + SAVE_EXT_MAX <= OVL_BTL_BASE,
       "the save extension area is not in the world scratch below the battle prefix")
ASSERT(SAVE_EXT_ADDR >= DOOA_SHARD_ADDR + DOOA_SHARD_SIZE, "the save extension area overlaps DOOA_SHARD")
/* MOD (save robustness): the evolution model area (0x4800 of texture, then the model) and the sound staging end below it */
ASSERT(EVO_MMD_ADDR + 0x4800 + PARTNER_MMD_MAX <= SAVE_EXT_ADDR, "the evolution model area overlaps the save extension area")
ASSERT(GENERAL_BUFFER_ADDR + SOUND_STAGING_MAX <= SAVE_EXT_ADDR, "the sound staging overlaps the save extension area")
ASSERT(STACK_SIZE_KB * 1024 >= 4 * STACK_MEASURED_MAX,
       "the stack is less than 4 times the deepest use measured")
ASSERT(main_BSS_END + MIN_HEAP <= STACK_ADDR - STACK_SIZE_KB * 1024,
       "main leaves less than MIN_HEAP bytes of heap below the stack")
