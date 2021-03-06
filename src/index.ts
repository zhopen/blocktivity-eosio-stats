import { PAUSE_MS } from "./config";
import { get_last_hour_block, get_hourly_counts } from "./get_hourly_counts";
import { get_block } from "./trace_api";
import { timeout, exists, save } from "./utils";
import { history } from "./history";

async function main() {
  const block_num = await get_last_hour_block();

  if ( !exists(block_num) ) {
    const block = await get_block( block_num );
    const hourly_counts = await get_hourly_counts( block ); // fetch hourly count data
    await save( block_num, hourly_counts ); // save locally as JSON
  } else {
    console.log(JSON.stringify({block_num, exists: true}));
  }
  await timeout(PAUSE_MS);
  await main();
}

(async () => {
  await history();
  await main();
})();