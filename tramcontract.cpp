#include "tramcontract.hpp"
#include <eosio.system/eosio.system.hpp>

using namespace ramtoken;

using eosiosystem::rammarket;
using sc = eosiosystem::system_contract;

void tramcontract::sell(asset quantity, name account) {

  // Burn TRAM tokens transfered by the user
  // implementar como funcion en vez de accion
  action(permission_level{ _self, "active"_n },
      TRAM.get_contract(), "retire"_n,
      std::make_tuple(quantity, std::string("TRAM burnt"))
  ).send();

  // Calculate how many EOS we will get by selling `quantity` bytes of RAM
  rammarket _rammarket("eosio"_n, "eosio"_n.value);
  auto market = _rammarket.get(sc::ramcore_symbol.raw(), "ram market does not exist");

  asset tokens_out;
  tokens_out = market.convert( asset(quantity.amount, sc::ram_symbol), sc::get_core_symbol());
  auto fee = ( tokens_out.amount + 199 ) / 200;
  tokens_out.amount -= fee;

  action(permission_level{ get_self(), "active"_n },
      "eosio"_n, "sellram"_n,
      std::make_tuple(_self, quantity.amount)
  ).send(); // chequear si esta función vende exactamente quantity o hace rodeo y termina aproximando.

  action(permission_level{ get_self(), "active"_n },
      "eosio.token"_n, "transfer"_n,
      std::make_tuple(_self, account, tokens_out, std::string("EOS tokens back"))
  ).send();
}

void tramcontract::buy(asset quantity, name account) {

  // Calculate how many bytes of RAM we will get by selling `quantity` EOS
  auto fee = quantity;
  fee.amount = ( fee.amount + 199 ) / 200;
  auto quant_after_fee = quantity;
  quant_after_fee.amount -= fee.amount;

  rammarket _rammarket("eosio"_n, "eosio"_n.value);
  auto market = _rammarket.get(sc::ramcore_symbol.raw(), "ram market does not exist");
  int64_t bytes_out = market.convert( quant_after_fee,  sc::ram_symbol ).amount;

  // Since the RAM to store the TRAM balance record will be deducted from this (self)
  // contract (if the user doesn't already have a TRAM balance), assert that bytes_out
  // is at least large enough to cover that cost (~250 bytes).
  accounts accountstable( TRAM.get_contract(), account.value );
  if (accountstable.find( TRAM.get_symbol().code().raw() ) == accountstable.end() ) {
    check( bytes_out >= 250, "user must buy at least 250 bytes");
  }

  // Buy ram from system market
  action(permission_level{ get_self(), "active"_n },
      "eosio"_n, "buyram"_n,
      std::make_tuple(get_self(), get_self(), quantity)
  ).send();

  // Issue bytes_out tokens to user
  // Implementar como funcion
  action(permission_level{ _self, "active"_n },
      TRAM.get_contract(), "issue"_n,
      std::make_tuple(account, asset{bytes_out, TRAM.get_symbol()}, std::string("TRAM issued"))
  ).send();

}

void tramcontract::ontransfer(name from, name to, asset quantity, string memo) {

  // A notification coming from `eosio.token`
  if( get_first_receiver() == "eosio.token"_n ) {
    check(sc::get_core_symbol() == quantity.symbol, "only EOS token allowed");

    // Having us as destination and not coming from eosio.ram we skip:
    //   - buy/sell ram transfer notifications
    //   - notifications of settlement transfers
    if (to == _self && from != "eosio.ram"_n)
      buy(quantity, from);
    return;
  }

  // A notification coming from `tramcontract`
  if ( get_first_receiver() == TRAM.get_contract() ) {

    if (from == _self) return;
    check(TRAM.get_symbol() == quantity.symbol, "only TRAM token allowed");
    check(to == _self, "only incoming TRAM transfers");

    settle(quantity, from);

    return;
  }

  check(false, "Invalid notification");
}