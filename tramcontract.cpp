#include "tramcontract.hpp"
#include <eosio.system/eosio.system.hpp>

using namespace ramtoken;

using eosiosystem::rammarket;
using sc = eosiosystem::system_contract;

void tramcontract::settle(asset quantity, name user) {

  // Burn TRAM tokens transfered by the user
  action(permission_level{ get_self(), "active"_n },
      get_self(), "retire"_n, std::make_tuple(quantity, std::string("TRAM burnt"))
  ).send();

  // Calculate how many core tokens we will get by selling 'quantity' bytes of RAM
  rammarket _rammarket("eosio"_n, "eosio"_n.value);
  auto market = _rammarket.get(sc::ramcore_symbol.raw(), "ram market does not exist");

  auto ram_balance = market.base.balance;
  auto core_balance = market.quote.balance;

  int64_t new_ram_bytes = 0;
  eosiosystem::global_state2_singleton state2("eosio"_n, "eosio"_n.value);
  if (state2.exists()) {
    auto state = state2.get();
    auto cbt = eosio::current_block_time();
    check( cbt > state.last_ram_increase, 
      "current block time must be larger than last_ram_increase"); // is this necessary?
    new_ram_bytes = (cbt.slot - state.last_ram_increase.slot)*state.new_ram_per_block;  
  }

  int64_t amount_out = eosiosystem::exchange_state::get_bancor_output(
    ram_balance.amount + new_ram_bytes, core_balance.amount, quantity.amount);  // inp_reserve, out_reserve, inp

  asset tokens_out = asset{amount_out, sc::get_core_symbol()};
  auto fee = ( tokens_out.amount + 199 ) / 200;
  tokens_out.amount -= fee;

  action(permission_level{ get_self(), "active"_n }, "eosio"_n, "sellram"_n,
      std::make_tuple(get_self(), quantity.amount)
  ).send();
  action(permission_level{ get_self(), "active"_n },"eosio.token"_n, "transfer"_n,
      std::make_tuple(get_self(), user, tokens_out, std::string("core tokens back"))
  ).send();
}

void tramcontract::buy(asset quantity, name user) {

  // Calculate how many bytes of RAM we will get by selling 'quantity' EOS
  auto fee = quantity;
  fee.amount = ( fee.amount + 199 ) / 200;
  auto quant_after_fee = quantity;
  quant_after_fee.amount -= fee.amount;

  rammarket _rammarket("eosio"_n, "eosio"_n.value);
  auto market = _rammarket.get(sc::ramcore_symbol.raw(), "ram market does not exist");
  auto ram_balance = market.base.balance;
  auto core_balance = market.quote.balance;

  int64_t new_ram_bytes = 0;
  eosiosystem::global_state2_singleton state2("eosio"_n, "eosio"_n.value);
  if (state2.exists()) {
    auto state = state2.get();
    auto cbt = eosio::current_block_time();
    check( cbt > state.last_ram_increase, 
      "current block time must be larger than last_ram_increase"); // is this necessary?
    new_ram_bytes = (cbt.slot - state.last_ram_increase.slot)*state.new_ram_per_block;  
  }

  int64_t bytes_out = eosiosystem::exchange_state::get_bancor_output(core_balance.amount,
    ram_balance.amount + new_ram_bytes, quant_after_fee.amount);  // inp_reserve, out_reserve, inp

  // Since the RAM to store the TRAM balance record will be deducted from this
  // contract whenever the user doesn't already have a TRAM balance, assert that bytes_out
  // is at least large enough to cover that cost (~250 bytes).
  accounts accountstable( get_self(), user.value );
  if (accountstable.find( TRAM.get_symbol().code().raw() ) == accountstable.end() ) {
    check( bytes_out >= 250, "user must buy at least 250 bytes");
  }

  // Buy ram from system market
  action(permission_level{ get_self(), "active"_n },
      "eosio"_n, "buyram"_n, std::make_tuple(get_self(), get_self(), quantity)
  ).send();

  // Issue bytes_out tokens to user
  action(permission_level{ get_self(), "active"_n }, get_self(), "issue"_n,
      std::make_tuple(get_self(), asset{bytes_out, TRAM.get_symbol()}, std::string("TRAM issued"))
  ).send();
  action(permission_level{ get_self(), "active"_n }, get_self(), "transfer"_n,
      std::make_tuple(get_self(), user, asset{bytes_out, TRAM.get_symbol()}, std::string("TRAM issued"))
  ).send();
}

void tramcontract::ontransfer(name from, name to, asset quantity, string memo) {

  // A notification coming from 'eosio.token'
  if( get_first_receiver() == "eosio.token"_n ) {
    check(sc::get_core_symbol() == quantity.symbol, "only core token allowed");
    if (to == get_self() && from != "eosio.ram"_n)
      buy(quantity, from);
  }

  // When ontransfer is called by this contract's transfer action
  if ( get_first_receiver() == get_self() ) {
    check(TRAM.get_symbol() == quantity.symbol, "only TRAM token allowed");
    settle(quantity, from);
  }
}