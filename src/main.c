#include "sbar.h"

#include "block/sblocks.h"

/** Example output
 * 
 * CPU 2%  RAM 1803/11722MiB(15%) |    5     100  | 03:03 28.05.2022 | dwm-6.3
 * 
 */

static sbar_t bar;

static sblock_t squotes;
static sblock_t ssysinfo;
static sblock_t swifi;
static sblock_t saudio;
static sblock_t smic;
static sblock_t sbattery;
static sblock_t stime;

int main()
{
    sbar_init(&bar);

    sblock_init(&squotes, squotes_routine);
    sblock_init(&ssysinfo, ssysinfo_routine);
    sblock_init(&swifi, swifi_routine);
    sblock_init(&saudio, saudio_routine);
    sblock_init(&smic, smic_routine);
    sblock_init(&sbattery, sbattery_routine);
    sblock_init(&stime, stime_routine);


    sbar_add_delim(&bar, " ");
    sbar_add_block(&bar, &squotes);
    sbar_add_delim(&bar, " | ");
    sbar_add_block(&bar, &ssysinfo);
    sbar_add_delim(&bar, " | ");
    sbar_add_block(&bar, &swifi);
    sbar_add_delim(&bar, "  ");
    sbar_add_block(&bar, &saudio);
    sbar_add_delim(&bar, "  ");
    sbar_add_block(&bar, &smic);
    sbar_add_delim(&bar, "  ");
    sbar_add_block(&bar, &sbattery);
    sbar_add_delim(&bar, " | ");
    sbar_add_block(&bar, &stime);
    sbar_add_delim(&bar, " | dwm-6.3 ");


    sbar_run(&bar);

    sbar_deinit(&bar);

    sblock_deinit(&squotes);
    sblock_deinit(&ssysinfo);
    sblock_deinit(&swifi);
    sblock_deinit(&saudio);
    sblock_deinit(&smic);
    sblock_deinit(&sbattery);
    sblock_deinit(&stime);
}
