import { Count } from "./interfaces";
import { rpc, ONE_HOUR, CONCURRENCY } from "./config";
import { timeout } from "./utils";
import PQueue from 'p-queue';
import { get_transaction_count } from "./get_transaction";
import moment from "moment";

// global timer
let before = moment.utc(moment.now()).unix();

export async function get_hourly_counts( start_block: number ) {
  before = moment.utc(moment.now()).unix();
  const hourly_counts: Count = {
    block_num: start_block,
    actions: 0,
    transactions: 0,
  }
  // queue up promises
  const queue = new PQueue({concurrency: CONCURRENCY});

  for (let i = start_block; i < start_block + ONE_HOUR; i++) {
    queue.add(async () => {
      const block_counts = await get_block_counts( i )
      hourly_counts.actions += block_counts.actions;
      hourly_counts.transactions += block_counts.transactions;

      // logging
      const after = moment.utc(moment.now()).unix();
      console.log(JSON.stringify({time: after - before, start_block, delta_num: ONE_HOUR - i % ONE_HOUR, block_counts}));
    });
  }

  // wait until queue is finished
  await queue.onIdle();

  // logging
  const after = moment.utc(moment.now()).unix();
  console.log(JSON.stringify({time: after - before, start_block, hourly_counts}));
  return hourly_counts;
}

export async function get_block_counts( block_num: number, retry = 3 ): Promise<Count> {

  let block: any;

  if (retry <= 0) {
    console.error("[ERROR] missing block", block_num);
    await timeout(5 * 1000); // pause for 5s
    return get_block_counts( block_num, 5 );
  }

  try {
    // get block info
    block = await rpc.get_block( block_num );
  } catch (e) {
    return get_block_counts( block_num, retry - 1 );
  }

  // store statistic counters
  const block_counts: Count = {
    block_num,
    actions: 0,
    transactions: 0,
  }

  // count each transaction
  for ( const { trx } of block.transactions ) {
    block_counts.transactions += 1;

    // full trace in block
    if (typeof(trx) == "object") {
      block_counts.actions += trx.transaction.actions.length;
    // traces executed by smart contract
    // must fetch individual transaction
    } else {
      block_counts.actions += await get_transaction_count( trx );
    }
  }
  return block_counts;
}

export async function get_last_hour_block(): Promise<number> {
  try {
    const { last_irreversible_block_num } = await rpc.get_info();

    // minus 1 hour & round down to the nearest 1 hour interval
    return (last_irreversible_block_num - ONE_HOUR) - last_irreversible_block_num % ONE_HOUR;
  } catch (e) {
    console.error("[ERROR] get info");
    timeout(5 * 1000) // pause for 5s
  }
  return get_last_hour_block();
}