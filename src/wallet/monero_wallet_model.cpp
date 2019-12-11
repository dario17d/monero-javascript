/**
 * Copyright (c) 2017-2019 woodser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Parts of this file are originally copyright (c) 2014-2019, The Monero Project
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers
 */

#include "monero_wallet_model.h"

#include "utils/gen_utils.h"
#include "utils/monero_utils.h"
#include <iostream>

using namespace std;

/**
 * Public library interface.
 */
namespace monero {

  // ----------------------- UNDECLARED PRIVATE HELPERS -----------------------

  void merge_incoming_transfer(vector<shared_ptr<monero_incoming_transfer>>& transfers, const shared_ptr<monero_incoming_transfer>& transfer) {
    for (const shared_ptr<monero_incoming_transfer>& aTransfer : transfers) {
      if (aTransfer->m_account_index.get() == transfer->m_account_index.get() && aTransfer->m_subaddress_index.get() == transfer->m_subaddress_index.get()) {
        aTransfer->merge(aTransfer, transfer);
        return;
      }
    }
    transfers.push_back(transfer);
  }

  shared_ptr<monero_block> node_to_block_query(const boost::property_tree::ptree& node) {
    shared_ptr<monero_block> block = make_shared<monero_block>();
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("txs")) {
        boost::property_tree::ptree txs_node = it->second;
        for (boost::property_tree::ptree::const_iterator it2 = txs_node.begin(); it2 != txs_node.end(); ++it2) {
          shared_ptr<monero_tx_query> tx_query = make_shared<monero_tx_query>();
          monero_tx_query::from_property_tree(it2->second, tx_query);
          block->m_txs.push_back(tx_query);
        }
      }
    }
    return block;
  }

  // -------------------------- MONERO SYNC RESULT ----------------------------

  rapidjson::Value monero_sync_result::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    monero_utils::addJsonMember("numBlocksFetched", m_num_blocks_fetched, allocator, root, value_num);

    // set bool values
    monero_utils::addJsonMember("receivedMoney", m_received_money, allocator, root);

    // return root
    return root;
  }

  // -------------------------- MONERO ACCOUNT -----------------------------

  rapidjson::Value monero_account::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_index != boost::none) monero_utils::addJsonMember("index", m_index.get(), allocator, root, value_num);
    if (m_balance != boost::none) monero_utils::addJsonMember("balance", m_balance.get(), allocator, root, value_num);
    if (m_unlocked_balance != boost::none) monero_utils::addJsonMember("unlockedBalance", m_unlocked_balance.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_primary_address != boost::none) monero_utils::addJsonMember("primaryAddress", m_primary_address.get(), allocator, root, value_str);
    if (m_tag != boost::none) monero_utils::addJsonMember("tag", m_tag.get(), allocator, root, value_str);

    // set subaddresses
    if (!m_subaddresses.empty()) root.AddMember("subaddresses", monero_utils::to_rapidjson_val(allocator, m_subaddresses), allocator);

    // return root
    return root;
  }

  // -------------------------- MONERO SUBADDRESS -----------------------------

  rapidjson::Value monero_subaddress::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_account_index != boost::none) monero_utils::addJsonMember("accountIndex", m_account_index.get(), allocator, root, value_num);
    if (m_index != boost::none) monero_utils::addJsonMember("index", m_index.get(), allocator, root, value_num);
    if (m_balance != boost::none) monero_utils::addJsonMember("balance", m_balance.get(), allocator, root, value_num);
    if (m_unlocked_balance != boost::none) monero_utils::addJsonMember("unlockedBalance", m_unlocked_balance.get(), allocator, root, value_num);
    if (m_num_unspent_outputs != boost::none) monero_utils::addJsonMember("numUnspentOutputs", m_num_unspent_outputs.get(), allocator, root, value_num);
    if (m_num_blocks_to_unlock) monero_utils::addJsonMember("numBlocksToUnlock", m_num_blocks_to_unlock.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);
    if (m_label != boost::none) monero_utils::addJsonMember("label", m_label.get(), allocator, root, value_str);

    // set bool values
    if (m_is_used != boost::none) monero_utils::addJsonMember("isUsed", m_is_used.get(), allocator, root);

    // return root
    return root;
  }

  // --------------------------- MONERO TX WALLET -----------------------------

  rapidjson::Value monero_tx_wallet::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_tx::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_input_sum != boost::none) monero_utils::addJsonMember("inputSum", m_input_sum.get(), allocator, root, value_num);
    if (m_output_sum != boost::none) monero_utils::addJsonMember("outputSum", m_output_sum.get(), allocator, root, value_num);
    if (m_change_amount != boost::none) monero_utils::addJsonMember("changeAmount", m_change_amount.get(), allocator, root, value_num);
    if (m_num_dummy_outputs != boost::none) monero_utils::addJsonMember("numDummyOutputs", m_num_dummy_outputs.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_note != boost::none) monero_utils::addJsonMember("note", m_note.get(), allocator, root, value_str);
    if (m_change_address != boost::none) monero_utils::addJsonMember("changeAddress", m_change_address.get(), allocator, root, value_str);
    if (m_extra_hex != boost::none) monero_utils::addJsonMember("extraHex", m_extra_hex.get(), allocator, root, value_str);

    // set bool values
    if (m_is_incoming != boost::none) monero_utils::addJsonMember("isIncoming", m_is_incoming.get(), allocator, root);
    if (m_is_outgoing != boost::none) monero_utils::addJsonMember("isOutgoing", m_is_outgoing.get(), allocator, root);
    if (m_is_locked != boost::none) monero_utils::addJsonMember("isLocked", m_is_locked.get(), allocator, root);

    // set sub-arrays
    if (!m_incoming_transfers.empty()) root.AddMember("incomingTransfers", monero_utils::to_rapidjson_val(allocator, m_incoming_transfers), allocator);

    // set sub-objects
    if (m_outgoing_transfer != boost::none) root.AddMember("outgoingTransfer", m_outgoing_transfer.get()->to_rapidjson_val(allocator), allocator);

    // return root
    return root;
  }

  void monero_tx_wallet::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_tx_wallet>& tx_wallet) {
    monero_tx::from_property_tree(node, tx_wallet);

    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      //if (key == string("hash")) tx->m_hash = it->second.data();
      if (key == string("isLocked")) tx_wallet->m_is_locked = it->second.get_value<bool>();
      // TODO: deserialize other fields
    }
  }

  shared_ptr<monero_tx_wallet> monero_tx_wallet::copy(const shared_ptr<monero_tx>& src, const shared_ptr<monero_tx>& tgt) const {
    MTRACE("monero_tx_wallet::copy(const shared_ptr<monero_tx>& src, const shared_ptr<monero_tx>& tgt)");
    return monero_tx_wallet::copy(static_pointer_cast<monero_tx_wallet>(src), static_pointer_cast<monero_tx_wallet>(tgt));
  };

  shared_ptr<monero_tx_wallet> monero_tx_wallet::copy(const shared_ptr<monero_tx_wallet>& src, const shared_ptr<monero_tx_wallet>& tgt) const {
    MTRACE("monero_tx_wallet::copy(const shared_ptr<monero_tx_wallet>& src, const shared_ptr<monero_tx_wallet>& tgt)");
    if (this != src.get()) throw runtime_error("this != src");

    // copy base class
    monero_tx::copy(static_pointer_cast<monero_tx>(src), static_pointer_cast<monero_tx>(tgt));

    // copy wallet extensions
    tgt->m_tx_set = src->m_tx_set;
    tgt->m_is_incoming = src->m_is_incoming;
    tgt->m_is_outgoing = src->m_is_outgoing;
    if (!src->m_incoming_transfers.empty()) {
      tgt->m_incoming_transfers = vector<shared_ptr<monero_incoming_transfer>>();
      for (const shared_ptr<monero_incoming_transfer>& transfer : src->m_incoming_transfers) {
        shared_ptr<monero_incoming_transfer> transferCopy = transfer->copy(transfer, make_shared<monero_incoming_transfer>());
        transferCopy->m_tx = tgt;
        tgt->m_incoming_transfers.push_back(transferCopy);
      }
    }
    if (src->m_outgoing_transfer != boost::none) {
      shared_ptr<monero_outgoing_transfer> transferCopy = src->m_outgoing_transfer.get()->copy(src->m_outgoing_transfer.get(), make_shared<monero_outgoing_transfer>());
      transferCopy->m_tx = tgt;
      tgt->m_outgoing_transfer = transferCopy;
    }
    tgt->m_note = src->m_note;
    tgt->m_is_locked = src->m_is_locked;
    tgt->m_input_sum = src->m_input_sum;
    tgt->m_output_sum = src->m_output_sum;
    tgt->m_change_address = src->m_change_address;
    tgt->m_change_amount = src->m_change_amount;
    tgt->m_num_dummy_outputs = src->m_num_dummy_outputs;
    tgt->m_extra_hex = src->m_extra_hex;

    return tgt;
  };

  void monero_tx_wallet::merge(const shared_ptr<monero_tx>& self, const shared_ptr<monero_tx>& other) {
    merge(static_pointer_cast<monero_tx_wallet>(self), static_pointer_cast<monero_tx_wallet>(other));
  }

  void monero_tx_wallet::merge(const shared_ptr<monero_tx_wallet>& self, const shared_ptr<monero_tx_wallet>& other) {
    if (this != self.get()) throw runtime_error("this != self");
    if (self == other) return;

    // merge base classes
    monero_tx::merge(self, other);

    // merge incoming transfers
    if (!other->m_incoming_transfers.empty()) {
      for (const shared_ptr<monero_incoming_transfer>& transfer : other->m_incoming_transfers) {  // NOTE: not using reference so shared_ptr is not deleted when tx is dereferenced
        transfer->m_tx = self;
        merge_incoming_transfer(self->m_incoming_transfers, transfer);
      }
    }

    // merge outgoing transfer
    if (other->m_outgoing_transfer != boost::none) {
      other->m_outgoing_transfer.get()->m_tx = self;
      if (self->m_outgoing_transfer == boost::none) self->m_outgoing_transfer = other->m_outgoing_transfer;
      else self->m_outgoing_transfer.get()->merge(self->m_outgoing_transfer.get(), other->m_outgoing_transfer.get());
    }

    // merge simple extensions
    m_is_incoming = gen_utils::reconcile(m_is_incoming, other->m_is_incoming);
    m_is_outgoing = gen_utils::reconcile(m_is_outgoing, other->m_is_outgoing);
    m_note = gen_utils::reconcile(m_note, other->m_note);
    m_is_locked = gen_utils::reconcile(m_is_locked, other->m_is_locked);
    m_input_sum = gen_utils::reconcile(m_input_sum, other->m_input_sum);
    m_output_sum = gen_utils::reconcile(m_output_sum, other->m_output_sum);
    m_change_address = gen_utils::reconcile(m_change_address, other->m_change_address);
    m_change_amount = gen_utils::reconcile(m_change_amount, other->m_change_amount);
    m_num_dummy_outputs = gen_utils::reconcile(m_num_dummy_outputs, other->m_num_dummy_outputs);
    m_extra_hex = gen_utils::reconcile(m_extra_hex, other->m_extra_hex);
  }

  // -------------------------- MONERO TX QUERY -----------------------------

  rapidjson::Value monero_tx_query::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_tx_wallet::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_height != boost::none) monero_utils::addJsonMember("height", m_height.get(), allocator, root, value_num);
    if (m_min_height != boost::none) monero_utils::addJsonMember("minHeight", m_min_height.get(), allocator, root, value_num);
    if (m_max_height != boost::none) monero_utils::addJsonMember("maxHeight", m_max_height.get(), allocator, root, value_num);

    // set bool values
    if (m_is_outgoing != boost::none) monero_utils::addJsonMember("isOutgoing", m_is_outgoing.get(), allocator, root);
    if (m_is_incoming != boost::none) monero_utils::addJsonMember("isIncoming", m_is_incoming.get(), allocator, root);
    if (m_has_payment_id != boost::none) monero_utils::addJsonMember("hasPaymentId", m_has_payment_id.get(), allocator, root);
    if (m_include_outputs != boost::none) monero_utils::addJsonMember("includeOutputs", m_include_outputs.get(), allocator, root);

    // set sub-arrays
    if (!m_tx_hashes.empty()) root.AddMember("txHashes", monero_utils::to_rapidjson_val(allocator, m_tx_hashes), allocator);
    if (!m_payment_ids.empty()) root.AddMember("paymentIds", monero_utils::to_rapidjson_val(allocator, m_payment_ids), allocator);

    // set sub-objects
    if (m_transfer_query != boost::none) root.AddMember("transferQuery", m_transfer_query.get()->to_rapidjson_val(allocator), allocator);

    // return root
    return root;
  }

  void monero_tx_query::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_tx_query>& tx_query) {
    monero_tx_wallet::from_property_tree(node, tx_query);

    // initialize query from node
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("isOutgoing")) tx_query->m_is_outgoing = it->second.get_value<bool>();
      else if (key == string("isIncoming")) tx_query->m_is_incoming = it->second.get_value<bool>();
      else if (key == string("txHashes")) for (boost::property_tree::ptree::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) tx_query->m_tx_hashes.push_back(it2->second.data());
      else if (key == string("hasPaymentId")) tx_query->m_has_payment_id = it->second.get_value<bool>();
      else if (key == string("paymentIds")) for (boost::property_tree::ptree::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) tx_query->m_payment_ids.push_back(it2->second.data());
      else if (key == string("height")) tx_query->m_height = it->second.get_value<uint64_t>();
      else if (key == string("minHeight")) tx_query->m_min_height = it->second.get_value<uint64_t>();
      else if (key == string("maxHeight")) tx_query->m_max_height = it->second.get_value<uint64_t>();
      else if (key == string("includeOutputs")) tx_query->m_include_outputs = it->second.get_value<bool>();
      else if (key == string("transferQuery")) {
        tx_query->m_transfer_query = make_shared<monero_transfer_query>();
        monero_transfer_query::from_property_tree(it->second, tx_query->m_transfer_query.get());
      }
      else if (key == string("outputQuery")) {
        tx_query->m_output_query = make_shared<monero_output_query>();
        monero_output_query::from_property_tree(it->second, tx_query->m_output_query.get());
      }
    }
  }

  shared_ptr<monero_tx_query> monero_tx_query::deserialize_from_block(const string& tx_query_json) {

    // deserialize tx query string to property rooted at block
    std::istringstream iss = tx_query_json.empty() ? std::istringstream() : std::istringstream(tx_query_json);
    boost::property_tree::ptree block_node;
    boost::property_tree::read_json(iss, block_node);

    // convert query property tree to block
    shared_ptr<monero_block> block = node_to_block_query(block_node);

    // get tx query
    shared_ptr<monero_tx_query> tx_query = static_pointer_cast<monero_tx_query>(block->m_txs[0]);

    // return deserialized query
    return tx_query;
  }

  shared_ptr<monero_tx_query> monero_tx_query::copy(const shared_ptr<monero_tx>& src, const shared_ptr<monero_tx>& tgt) const {
    return copy(static_pointer_cast<monero_tx_query>(src), static_pointer_cast<monero_tx_query>(tgt));
  };

  shared_ptr<monero_tx_query> monero_tx_query::copy(const shared_ptr<monero_tx_wallet>& src, const shared_ptr<monero_tx_wallet>& tgt) const {
    return copy(static_pointer_cast<monero_tx_query>(src), static_pointer_cast<monero_tx_query>(tgt));
  };

  shared_ptr<monero_tx_query> monero_tx_query::copy(const shared_ptr<monero_tx_query>& src, const shared_ptr<monero_tx_query>& tgt) const {
    MTRACE("monero_tx_query::copy(const shared_ptr<monero_tx_query>& src, const shared_ptr<monero_tx_query>& tgt)");
    if (this != src.get()) throw runtime_error("this != src");

    // copy base class
    monero_tx_wallet::copy(static_pointer_cast<monero_tx>(src), static_pointer_cast<monero_tx>(tgt));

    // copy query extensions
    tgt->m_is_outgoing = src->m_is_outgoing;
    tgt->m_is_incoming = src->m_is_incoming;
    if (!src->m_tx_hashes.empty()) tgt->m_tx_hashes = vector<string>(src->m_tx_hashes);
    tgt-> m_has_payment_id = src->m_has_payment_id;
    if (!src->m_payment_ids.empty()) tgt->m_payment_ids = vector<string>(src->m_payment_ids);
    tgt->m_height = src->m_height;
    tgt->m_min_height = src->m_min_height;
    tgt->m_max_height = src->m_max_height;
    tgt->m_include_outputs = src->m_include_outputs;
    if (src->m_transfer_query != boost::none) tgt->m_transfer_query = src->m_transfer_query.get()->copy(src->m_transfer_query.get(), make_shared<monero_transfer_query>());
    if (src->m_output_query != boost::none) tgt->m_output_query = src->m_output_query.get()->copy(src->m_output_query.get(), make_shared<monero_output_query>());
    return tgt;
  };

  bool monero_tx_query::meets_criteria(monero_tx_wallet* tx) const {
    if (tx == nullptr) return false;

    // filter on tx
    if (m_hash != boost::none && m_hash != tx->m_hash) return false;
    if (m_payment_id != boost::none && m_payment_id != tx->m_payment_id) return false;
    if (m_is_confirmed != boost::none && m_is_confirmed != tx->m_is_confirmed) return false;
    if (m_in_tx_pool != boost::none && m_in_tx_pool != tx->m_in_tx_pool) return false;
    if (m_do_not_relay != boost::none && m_do_not_relay != tx->m_do_not_relay) return false;
    if (m_is_failed != boost::none && m_is_failed != tx->m_is_failed) return false;
    if (m_is_miner_tx != boost::none && m_is_miner_tx != tx->m_is_miner_tx) return false;
    if (m_is_locked != boost::none && m_is_locked != tx->m_is_locked) return false;

    // at least one transfer must meet transfer query if defined
    if (m_transfer_query != boost::none) {
      bool matchFound = false;
      if (tx->m_outgoing_transfer != boost::none && m_transfer_query.get()->meets_criteria(tx->m_outgoing_transfer.get().get())) matchFound = true;
      else if (!tx->m_incoming_transfers.empty()) {
        for (const shared_ptr<monero_incoming_transfer>& incoming_transfer : tx->m_incoming_transfers) {
          if (m_transfer_query.get()->meets_criteria(incoming_transfer.get())) {
            matchFound = true;
            break;
          }
        }
      }
      if (!matchFound) return false;
    }

    // at least one output must meet output query if defined
    if (m_output_query != boost::none && !m_output_query.get()->is_default()) {
      if (tx->m_outputs.empty()) return false;
      bool matchFound = false;
      for (const shared_ptr<monero_output>& output : tx->m_outputs) {
        shared_ptr<monero_output_wallet> vout_wallet = static_pointer_cast<monero_output_wallet>(output);
        if (m_output_query.get()->meets_criteria(vout_wallet.get())) {
          matchFound = true;
          break;
        }
      }
      if (!matchFound) return false;
    }

    // filter on having a payment id
    if (m_has_payment_id != boost::none) {
      if (*m_has_payment_id && tx->m_payment_id == boost::none) return false;
      if (!*m_has_payment_id && tx->m_payment_id != boost::none) return false;
    }

    // filter on incoming
    if (m_is_incoming != boost::none && m_is_incoming != tx->m_is_incoming) return false;

    // filter on outgoing
    if (m_is_outgoing != boost::none && m_is_outgoing != tx->m_is_outgoing) return false;

    // filter on remaining fields
    boost::optional<uint64_t> txHeight = tx->get_height();
    if (!m_tx_hashes.empty() && find(m_tx_hashes.begin(), m_tx_hashes.end(), *tx->m_hash) == m_tx_hashes.end()) return false;
    if (!m_payment_ids.empty() && (tx->m_payment_id == boost::none || find(m_payment_ids.begin(), m_payment_ids.end(), *tx->m_payment_id) == m_payment_ids.end())) return false;
    if (m_height != boost::none && (txHeight == boost::none || *txHeight != *m_height)) return false;
    if (m_min_height != boost::none && (txHeight == boost::none || *txHeight < *m_min_height)) return false;
    if (m_max_height != boost::none && (txHeight == boost::none || *txHeight > *m_max_height)) return false;

    // transaction meets query criteria
    return true;
  }

  // -------------------------- MONERO DESTINATION ----------------------------

  rapidjson::Value monero_destination::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_amount != boost::none) monero_utils::addJsonMember("amount", m_amount.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);

    // return root
    return root;
  }

  void monero_destination::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_destination>& destination) {
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("address")) destination->m_address = it->second.data();
      else if (key == string("amount")) destination->m_amount = it->second.get_value<uint64_t>();
    }
  }

  shared_ptr<monero_destination> monero_destination::copy(const shared_ptr<monero_destination>& src, const shared_ptr<monero_destination>& tgt) const {
    MTRACE("monero_destination::copy(const shared_ptr<monero_destination>& src, const shared_ptr<monero_destination>& tgt)");
    if (this != src.get()) throw runtime_error("this != src");
    tgt->m_address = src->m_address;
    tgt->m_amount = src->m_amount;
    return tgt;
  };

  // ----------------------------- MONERO TX SET ------------------------------

  rapidjson::Value monero_tx_set::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_multisig_tx_hex != boost::none) monero_utils::addJsonMember("multisigTxHex", m_multisig_tx_hex.get(), allocator, root, value_str);
    if (m_unsigned_tx_hex != boost::none) monero_utils::addJsonMember("unsignedTxHex", m_unsigned_tx_hex.get(), allocator, root, value_str);
    if (m_signed_tx_hex != boost::none) monero_utils::addJsonMember("signedTxHex", m_signed_tx_hex.get(), allocator, root, value_str);

    // set sub-arrays
    if (!m_txs.empty()) root.AddMember("txs", monero_utils::to_rapidjson_val(allocator, m_txs), allocator);

    // return root
    return root;
  }

  monero_tx_set monero_tx_set::deserialize(const string& tx_set_json) {

    // deserialize tx set to property
    std::istringstream iss = tx_set_json.empty() ? std::istringstream() : std::istringstream(tx_set_json);
    boost::property_tree::ptree tx_set_node;
    boost::property_tree::read_json(iss, tx_set_node);

    // initialize tx_set from property node
    monero_tx_set tx_set;
    for (boost::property_tree::ptree::const_iterator it = tx_set_node.begin(); it != tx_set_node.end(); ++it) {
      string key = it->first;
      if (key == string("unsignedTxHex")) tx_set.m_unsigned_tx_hex = it->second.data();
      else if (key == string("multisigTxHex")) tx_set.m_multisig_tx_hex = it->second.data();
      else if (key == string("txs")) {
        boost::property_tree::ptree txs_node = it->second;
        for (boost::property_tree::ptree::const_iterator it2 = txs_node.begin(); it2 != txs_node.end(); ++it2) {
          shared_ptr<monero_tx_wallet> tx_wallet = make_shared<monero_tx_wallet>();
          monero_tx_wallet::from_property_tree(it2->second, tx_wallet);
          tx_set.m_txs.push_back(tx_wallet);
        }
      }
      else throw runtime_error("monero_utils::deserialize_tx_set() field '" + key + "' not supported");
    }

    return tx_set;
  }

  // ---------------------------- MONERO TRANSFER -----------------------------

  rapidjson::Value monero_transfer::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_amount != boost::none) monero_utils::addJsonMember("amount", m_amount.get(), allocator, root, value_num);
    if (m_account_index != boost::none) monero_utils::addJsonMember("accountIndex", m_account_index.get(), allocator, root, value_num);
    if (m_num_suggested_confirmations != boost::none) monero_utils::addJsonMember("numSuggestedConfirmations", m_num_suggested_confirmations.get(), allocator, root, value_num);

    // return root
    return root;
  }

  void monero_transfer::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_transfer>& transfer) {

    // initialize transfer from node
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("accountIndex")) transfer->m_account_index = it->second.get_value<uint32_t>();
    }
  }

  shared_ptr<monero_transfer> monero_transfer::copy(const shared_ptr<monero_transfer>& src, const shared_ptr<monero_transfer>& tgt) const {
    MTRACE("monero_transfer::copy(const shared_ptr<monero_transfer>& src, const shared_ptr<monero_transfer>& tgt)");
    tgt->m_tx = src->m_tx;  // reference parent tx by default
    tgt->m_amount = src->m_amount;
    tgt->m_account_index = src->m_account_index;
    tgt->m_num_suggested_confirmations = src->m_num_suggested_confirmations;
    return tgt;
  }

  void monero_transfer::merge(const shared_ptr<monero_transfer>& self, const shared_ptr<monero_transfer>& other) {
    if (this != self.get()) throw runtime_error("this != self");
    if (self == other) return;

    // merge txs if they're different which comes back to merging transfers
    if (m_tx != other->m_tx) {
      m_tx->merge(m_tx, other->m_tx);
      return;
    }

    // otherwise merge transfer fields
    m_account_index = gen_utils::reconcile(m_account_index, other->m_account_index, "acountIndex");

    // TODO monero core: failed m_tx in pool (after testUpdateLockedDifferentAccounts()) causes non-originating saved wallets to return duplicate incoming transfers but one has amount/m_num_suggested_confirmations of 0
    if (m_amount != boost::none && other->m_amount != boost::none && *m_amount != *other->m_amount && (*m_amount == 0 || *other->m_amount == 0)) {
      m_account_index = gen_utils::reconcile(m_account_index, other->m_account_index, boost::none, boost::none, true, "acountIndex");
      m_num_suggested_confirmations = gen_utils::reconcile(m_num_suggested_confirmations, other->m_num_suggested_confirmations, boost::none, boost::none, true, "m_num_suggested_confirmations");
      MWARNING("WARNING: failed tx in pool causes non-originating wallets to return duplicate incoming transfers but with one amount/m_num_suggested_confirmations of 0");
    } else {
      m_amount = gen_utils::reconcile(m_amount, other->m_amount, "transfer amount");
      m_num_suggested_confirmations = gen_utils::reconcile(m_num_suggested_confirmations, other->m_num_suggested_confirmations, boost::none, boost::none, false, "m_num_suggested_confirmations");
    }
  }

  // ----------------------- MONERO INCOMING TRANSFER -------------------------

  rapidjson::Value monero_incoming_transfer::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_transfer::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_subaddress_index != boost::none) monero_utils::addJsonMember("subaddressIndex", m_subaddress_index.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);

    // return root
    return root;
  }

  shared_ptr<monero_incoming_transfer> monero_incoming_transfer::copy(const shared_ptr<monero_transfer>& src, const shared_ptr<monero_transfer>& tgt) const {
    return copy(static_pointer_cast<monero_incoming_transfer>(src), static_pointer_cast<monero_incoming_transfer>(tgt));
  }

  shared_ptr<monero_incoming_transfer> monero_incoming_transfer::copy(const shared_ptr<monero_incoming_transfer>& src, const shared_ptr<monero_incoming_transfer>& tgt) const {
    cout << "monero_incoming_transfer::copy()" << endl;
    throw runtime_error("monero_incoming_transfer::copy(inTransfer) not implemented");
  };

  boost::optional<bool> monero_incoming_transfer::is_incoming() const { return true; }

  void monero_incoming_transfer::merge(const shared_ptr<monero_transfer>& self, const shared_ptr<monero_transfer>& other) {
    merge(static_pointer_cast<monero_incoming_transfer>(self), static_pointer_cast<monero_incoming_transfer>(other));
  }

  void monero_incoming_transfer::merge(const shared_ptr<monero_incoming_transfer>& self, const shared_ptr<monero_incoming_transfer>& other) {
    if (self == other) return;
    monero_transfer::merge(self, other);
    m_subaddress_index = gen_utils::reconcile(m_subaddress_index, other->m_subaddress_index, "incoming transfer m_subaddress_index");
    m_address = gen_utils::reconcile(m_address, other->m_address);
  }

  // ----------------------- MONERO OUTGOING TRANSFER -------------------------

  rapidjson::Value monero_outgoing_transfer::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_transfer::to_rapidjson_val(allocator);

    // set sub-arrays
    if (!m_subaddress_indices.empty()) root.AddMember("subaddressIndices", monero_utils::to_rapidjson_val(allocator, m_subaddress_indices), allocator);
    if (!m_addresses.empty()) root.AddMember("addresses", monero_utils::to_rapidjson_val(allocator, m_addresses), allocator);
    if (!m_destinations.empty()) root.AddMember("destinations", monero_utils::to_rapidjson_val(allocator, m_destinations), allocator);

    // return root
    return root;
  }

  shared_ptr<monero_outgoing_transfer> monero_outgoing_transfer::copy(const shared_ptr<monero_transfer>& src, const shared_ptr<monero_transfer>& tgt) const {
    return copy(static_pointer_cast<monero_outgoing_transfer>(src), static_pointer_cast<monero_outgoing_transfer>(tgt));
  };

  shared_ptr<monero_outgoing_transfer> monero_outgoing_transfer::copy(const shared_ptr<monero_outgoing_transfer>& src, const shared_ptr<monero_outgoing_transfer>& tgt) const {
    cout << "monero_outgoing_transfer::copy()" << endl;
    throw runtime_error("monero_outgoing_transfer::copy(out_transfer) not implemented");
  };

  boost::optional<bool> monero_outgoing_transfer::is_incoming() const { return false; }

  void monero_outgoing_transfer::merge(const shared_ptr<monero_transfer>& self, const shared_ptr<monero_transfer>& other) {
    merge(static_pointer_cast<monero_outgoing_transfer>(self), static_pointer_cast<monero_outgoing_transfer>(other));
  }

  void monero_outgoing_transfer::merge(const shared_ptr<monero_outgoing_transfer>& self, const shared_ptr<monero_outgoing_transfer>& other) {
    if (self == other) return;
    monero_transfer::merge(self, other);
    m_subaddress_indices = gen_utils::reconcile(m_subaddress_indices, other->m_subaddress_indices);
    m_addresses = gen_utils::reconcile(m_addresses, other->m_addresses);
    m_destinations = gen_utils::reconcile(m_destinations, other->m_destinations);
  }

  // ----------------------- MONERO TRANSFER REQUEST --------------------------

  rapidjson::Value monero_transfer_query::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_transfer::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_subaddress_index != boost::none) monero_utils::addJsonMember("subaddressIndex", m_subaddress_index.get(), allocator, root, value_num);

    // set bool values
    if (m_is_incoming != boost::none) monero_utils::addJsonMember("isIncoming", m_is_incoming.get(), allocator, root);
    if (m_has_destinations != boost::none) monero_utils::addJsonMember("hasDestinations", m_has_destinations.get(), allocator, root);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);

    // set sub-arrays
    if (!m_subaddress_indices.empty()) root.AddMember("subaddressIndices", monero_utils::to_rapidjson_val(allocator, m_subaddress_indices), allocator);
    if (!m_addresses.empty()) root.AddMember("addresses", monero_utils::to_rapidjson_val(allocator, m_addresses), allocator);
    if (!m_destinations.empty()) root.AddMember("destinations", monero_utils::to_rapidjson_val(allocator, m_destinations), allocator);

    // return root
    return root;
  }

  void monero_transfer_query::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_transfer_query>& transfer_query) {
    monero_transfer::from_property_tree(node, transfer_query);

    // initialize query from node
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("isIncoming")) transfer_query->m_is_incoming = it->second.get_value<bool>();
      else if (key == string("address")) transfer_query->m_address = it->second.data();
      else if (key == string("addresses")) throw runtime_error("addresses not implemented");
      else if (key == string("subaddressIndex")) transfer_query->m_subaddress_index = it->second.get_value<uint32_t>();
      else if (key == string("subaddressIndices")) {
        vector<uint32_t> m_subaddress_indices;
        for (const auto& child : it->second) m_subaddress_indices.push_back(child.second.get_value<uint32_t>());
        transfer_query->m_subaddress_indices = m_subaddress_indices;
      }
      else if (key == string("destinations")) throw runtime_error("destinations not implemented");
      else if (key == string("hasDestinations")) transfer_query->m_has_destinations = it->second.get_value<bool>();
      else if (key == string("txQuery")) throw runtime_error("txQuery not implemented");
    }
  }

  shared_ptr<monero_transfer_query> monero_transfer_query::deserialize_from_block(const string& transfer_query_json) {

    // deserialize transfer query string to property rooted at block
    std::istringstream iss = transfer_query_json.empty() ? std::istringstream() : std::istringstream(transfer_query_json);
    boost::property_tree::ptree blockNode;
    boost::property_tree::read_json(iss, blockNode);

    // convert query property tree to block
    shared_ptr<monero_block> block = node_to_block_query(blockNode);

    // return empty query if no txs
    if (block->m_txs.empty()) return make_shared<monero_transfer_query>();

    // get tx query
    shared_ptr<monero_tx_query> tx_query = static_pointer_cast<monero_tx_query>(block->m_txs[0]);

    // get / create transfer query
    shared_ptr<monero_transfer_query> transfer_query = tx_query->m_transfer_query == boost::none ? make_shared<monero_transfer_query>() : *tx_query->m_transfer_query;

    // transfer query references tx query but not the other way around to avoid circular loop // TODO: could add check within meetsCriterias()
    transfer_query->m_tx_query = tx_query;
    tx_query->m_transfer_query = boost::none;

    // return deserialized query
    return transfer_query;
  }

  shared_ptr<monero_transfer_query> monero_transfer_query::copy(const shared_ptr<monero_transfer>& src, const shared_ptr<monero_transfer>& tgt) const {
    return copy(static_pointer_cast<monero_transfer_query>(src), static_pointer_cast<monero_transfer>(tgt));
  };

  shared_ptr<monero_transfer_query> monero_transfer_query::copy(const shared_ptr<monero_transfer_query>& src, const shared_ptr<monero_transfer_query>& tgt) const {
    MTRACE("monero_transfer_query::copy(const shared_ptr<monero_transfer_query>& src, const shared_ptr<monero_transfer_query>& tgt)");
    if (this != src.get()) throw runtime_error("this != src");

    // copy base class
    monero_transfer::copy(static_pointer_cast<monero_transfer>(src), static_pointer_cast<monero_transfer>(tgt));

    // copy extensions
    tgt->m_is_incoming = src->m_is_incoming;
    tgt->m_address = src->m_address;
    if (!src->m_addresses.empty()) tgt->m_addresses = vector<string>(src->m_addresses);
    tgt->m_subaddress_index = src->m_subaddress_index;
    if (!src->m_subaddress_indices.empty()) tgt->m_subaddress_indices = vector<uint32_t>(src->m_subaddress_indices);
    if (!src->m_destinations.empty()) {
      for (const shared_ptr<monero_destination>& destination : src->m_destinations) {
        tgt->m_destinations.push_back(destination->copy(destination, make_shared<monero_destination>()));
      }
    }
    tgt->m_has_destinations = src->m_has_destinations;
    tgt->m_tx_query = src->m_tx_query;
    return tgt;
  };

  bool monero_transfer_query::meets_criteria(monero_transfer* transfer) const {
    if (transfer == nullptr) throw runtime_error("transfer is null");
    if (m_tx_query != boost::none && (*m_tx_query)->m_transfer_query != boost::none) throw runtime_error("Transfer query's tx query cannot have a circular transfer query");   // TODO: could auto detect and handle this.  port to java/js

    // filter on common fields
    if (is_incoming() != boost::none && *is_incoming() != *transfer->is_incoming()) return false;
    if (is_outgoing() != boost::none && is_outgoing() != transfer->is_outgoing()) return false;
    if (m_amount != boost::none && *m_amount != *transfer->m_amount) return false;
    if (m_account_index != boost::none && *m_account_index != *transfer->m_account_index) return false;

    // filter on incoming fields
    monero_incoming_transfer* inTransfer = dynamic_cast<monero_incoming_transfer*>(transfer);
    if (inTransfer != nullptr) {
      if (m_has_destinations != boost::none) return false;
      if (m_address != boost::none && *m_address != *inTransfer->m_address) return false;
      if (!m_addresses.empty() && find(m_addresses.begin(), m_addresses.end(), *inTransfer->m_address) == m_addresses.end()) return false;
      if (m_subaddress_index != boost::none && *m_subaddress_index != *inTransfer->m_subaddress_index) return false;
      if (!m_subaddress_indices.empty() && find(m_subaddress_indices.begin(), m_subaddress_indices.end(), *inTransfer->m_subaddress_index) == m_subaddress_indices.end()) return false;
    }

    // filter on outgoing fields
    monero_outgoing_transfer* out_transfer = dynamic_cast<monero_outgoing_transfer*>(transfer);
    if (out_transfer != nullptr) {

      // filter on addresses
      if (m_address != boost::none && (out_transfer->m_addresses.empty() || find(out_transfer->m_addresses.begin(), out_transfer->m_addresses.end(), *m_address) == out_transfer->m_addresses.end())) return false;   // TODO: will filter all transfers if they don't contain addresses
      if (!m_addresses.empty()) {
        bool intersects = false;
        for (const string& addressReq : m_addresses) {
          for (const string& address : out_transfer->m_addresses) {
            if (addressReq == address) {
              intersects = true;
              break;
            }
          }
        }
        if (!intersects) return false;  // must have overlapping addresses
      }

      // filter on subaddress indices
      if (m_subaddress_index != boost::none && (out_transfer->m_subaddress_indices.empty() || find(out_transfer->m_subaddress_indices.begin(), out_transfer->m_subaddress_indices.end(), *m_subaddress_index) == out_transfer->m_subaddress_indices.end())) return false;   // TODO: will filter all transfers if they don't contain subaddress indices
      if (!m_subaddress_indices.empty()) {
        bool intersects = false;
        for (const uint32_t& subaddressIndexReq : m_subaddress_indices) {
          for (const uint32_t& m_subaddress_index : out_transfer->m_subaddress_indices) {
            if (subaddressIndexReq == m_subaddress_index) {
              intersects = true;
              break;
            }
          }
        }
        if (!intersects) return false;  // must have overlapping subaddress indices
      }

      // filter on having destinations
      if (m_has_destinations != boost::none) {
        if (*m_has_destinations && out_transfer->m_destinations.empty()) return false;
        if (!*m_has_destinations && !out_transfer->m_destinations.empty()) return false;
      }

      // filter on destinations TODO: start with test for this
      //    if (this.getDestionations() != null && this.getDestionations() != transfer.getDestionations()) return false;
    }

    // validate type
    if (inTransfer == nullptr && out_transfer == nullptr) throw runtime_error("Transfer must be monero_incoming_transfer or monero_outgoing_transfer");

    // filter with tx query
    if (m_tx_query != boost::none && !(*m_tx_query)->meets_criteria(transfer->m_tx.get())) return false;
    return true;
  }

  boost::optional<bool> monero_transfer_query::is_incoming() const { return m_is_incoming; }

  // ------------------------- MONERO OUTPUT WALLET ---------------------------

  rapidjson::Value monero_output_wallet::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_output::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_account_index != boost::none) monero_utils::addJsonMember("accountIndex", m_account_index.get(), allocator, root, value_num);
    if (m_subaddress_index != boost::none) monero_utils::addJsonMember("subaddressIndex", m_subaddress_index.get(), allocator, root, value_num);

    // set bool values
    if (m_is_spent != boost::none) monero_utils::addJsonMember("isSpent", m_is_spent.get(), allocator, root);
    if (m_is_frozen != boost::none) monero_utils::addJsonMember("isFrozen", m_is_frozen.get(), allocator, root);

    // return root
    return root;
  }

  void monero_output_wallet::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_output_wallet>& output_wallet) {
    monero_output::from_property_tree(node, output_wallet);
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("accountIndex")) output_wallet->m_account_index = it->second.get_value<uint32_t>();
      else if (key == string("subaddressIndex")) output_wallet->m_subaddress_index = it->second.get_value<uint32_t>();
      else if (key == string("isSpent")) output_wallet->m_is_spent = it->second.get_value<bool>();
      else if (key == string("isFrozen")) output_wallet->m_is_frozen = it->second.get_value<bool>();
    }
  }

  shared_ptr<monero_output_wallet> monero_output_wallet::copy(const shared_ptr<monero_output>& src, const shared_ptr<monero_output>& tgt) const {
    MTRACE("monero_output_wallet::copy(output)");
    return monero_output_wallet::copy(static_pointer_cast<monero_output_wallet>(src), static_pointer_cast<monero_output_wallet>(tgt));
  };

  shared_ptr<monero_output_wallet> monero_output_wallet::copy(const shared_ptr<monero_output_wallet>& src, const shared_ptr<monero_output_wallet>& tgt) const {
    MTRACE("monero_output_wallet::copy(output_wallet)");
    if (this != src.get()) throw runtime_error("this != src");

    // copy base class
    monero_output::copy(static_pointer_cast<monero_output>(src), static_pointer_cast<monero_output>(tgt));

    // copy extensions
    tgt->m_account_index = src->m_account_index;
    tgt->m_subaddress_index = src->m_subaddress_index;
    tgt->m_is_spent = src->m_is_spent;
    tgt->m_is_frozen = src->m_is_frozen;
    return tgt;
  };

  void monero_output_wallet::merge(const shared_ptr<monero_output>& self, const shared_ptr<monero_output>& other) {
    merge(static_pointer_cast<monero_output_wallet>(self), static_pointer_cast<monero_output_wallet>(other));
  }

  void monero_output_wallet::merge(const shared_ptr<monero_output_wallet>& self, const shared_ptr<monero_output_wallet>& other) {
    MTRACE("monero_output_wallet::merge(self, other)");
    if (this != self.get()) throw runtime_error("this != self");
    if (self == other) return;

    // merge base classes
    monero_output::merge(self, other);

    // merge output wallet extensions
    m_account_index = gen_utils::reconcile(m_account_index, other->m_account_index);
    m_subaddress_index = gen_utils::reconcile(m_subaddress_index, other->m_subaddress_index);
    m_is_spent = gen_utils::reconcile(m_is_spent, other->m_is_spent);
    m_is_frozen = gen_utils::reconcile(m_is_frozen, other->m_is_frozen);
  }

  // ------------------------ MONERO OUTPUT REQUEST ---------------------------

  rapidjson::Value monero_output_query::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_output_wallet::to_rapidjson_val(allocator);

    // set sub-arrays
    if (!m_subaddress_indices.empty()) root.AddMember("subaddressIndices", monero_utils::to_rapidjson_val(allocator, m_subaddress_indices), allocator);

    // return root
    return root;
  }

  void monero_output_query::from_property_tree(const boost::property_tree::ptree& node, const shared_ptr<monero_output_query>& output_query) {
    monero_output_wallet::from_property_tree(node, output_query);

    // initialize query from node
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("subaddressIndices")) for (boost::property_tree::ptree::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) output_query->m_subaddress_indices.push_back(it2->second.get_value<uint32_t>());
      else if (key == string("txQuery")) {} // ignored
    }
  }

  shared_ptr<monero_output_query> monero_output_query::deserialize_from_block(const string& output_query_json) {

    // deserialize output query string to property rooted at block
    std::istringstream iss = output_query_json.empty() ? std::istringstream() : std::istringstream(output_query_json);
    boost::property_tree::ptree blockNode;
    boost::property_tree::read_json(iss, blockNode);

    // convert query property tree to block
    shared_ptr<monero_block> block = node_to_block_query(blockNode);

    // empty query if no txs
    if (block->m_txs.empty()) return make_shared<monero_output_query>();

    // get tx query
    shared_ptr<monero_tx_query> tx_query = static_pointer_cast<monero_tx_query>(block->m_txs[0]);

    // get / create output query
    shared_ptr<monero_output_query> output_query = tx_query->m_output_query == boost::none ? make_shared<monero_output_query>() : *tx_query->m_output_query;

    // output query references tx query but not the other way around to avoid circular loop // TODO: could add check within meetsCriterias(), TODO: move this temp hack to the caller
    output_query->m_tx_query = tx_query;
    tx_query->m_output_query = boost::none;

    // return deserialized query
    return output_query;
  }

  // initialize static empty output for is_default() check
  const unique_ptr<monero_output_wallet> monero_output_query::M_EMPTY_OUTPUT = unique_ptr<monero_output_wallet>(new monero_output_wallet());
  bool monero_output_query::is_default() const {
    return meets_criteria(monero_output_query::M_EMPTY_OUTPUT.get());
  }

  shared_ptr<monero_output_query> monero_output_query::copy(const shared_ptr<monero_output>& src, const shared_ptr<monero_output>& tgt) const {
    return monero_output_query::copy(static_pointer_cast<monero_output_query>(src), static_pointer_cast<monero_output_query>(tgt));
  };

  shared_ptr<monero_output_query> monero_output_query::copy(const shared_ptr<monero_output_wallet>& src, const shared_ptr<monero_output_wallet>& tgt) const {
    return monero_output_query::copy(static_pointer_cast<monero_output_query>(src), static_pointer_cast<monero_output_query>(tgt));
  };

  shared_ptr<monero_output_query> monero_output_query::copy(const shared_ptr<monero_output_query>& src, const shared_ptr<monero_output_query>& tgt) const {
    MTRACE("monero_output_query::copy(output_query)");
    if (this != src.get()) throw runtime_error("this != src");

    // copy base class
    monero_output_wallet::copy(static_pointer_cast<monero_output>(src), static_pointer_cast<monero_output>(tgt));

    // copy extensions
    if (!src->m_subaddress_indices.empty()) tgt->m_subaddress_indices = vector<uint32_t>(src->m_subaddress_indices);
    return tgt;
  };

  bool monero_output_query::meets_criteria(monero_output_wallet* output) const {

    // filter on output
    if (m_account_index != boost::none && (output->m_account_index == boost::none || *m_account_index != *output->m_account_index)) return false;
    if (m_subaddress_index != boost::none && (output->m_subaddress_index == boost::none || *m_subaddress_index != *output->m_subaddress_index)) return false;
    if (m_amount != boost::none && (output->m_amount == boost::none || *m_amount != *output->m_amount)) return false;
    if (m_is_spent != boost::none && (output->m_is_spent == boost::none || *m_is_spent != *output->m_is_spent)) return false;

    // filter on output key image
    if (m_key_image != boost::none) {
      if (output->m_key_image == boost::none) return false;
      if ((*m_key_image)->m_hex != boost::none && ((*output->m_key_image)->m_hex == boost::none || *(*m_key_image)->m_hex != *(*output->m_key_image)->m_hex)) return false;
      if ((*m_key_image)->m_signature != boost::none && ((*output->m_key_image)->m_signature == boost::none || *(*m_key_image)->m_signature != *(*output->m_key_image)->m_signature)) return false;
    }

    // filter on extensions
    if (!m_subaddress_indices.empty() && find(m_subaddress_indices.begin(), m_subaddress_indices.end(), *output->m_subaddress_index) == m_subaddress_indices.end()) return false;

    // filter with tx query
    if (m_tx_query != boost::none && !(*m_tx_query)->meets_criteria(static_pointer_cast<monero_tx_wallet>(output->m_tx).get())) return false;

    // output meets query
    return true;
  }

  // ------------------------- MONERO SEND REQUEST ----------------------------

  monero_send_request::monero_send_request(const monero_send_request& request) {
    if (request.m_destinations.size() > 0) {
      for (const shared_ptr<monero_destination>& destination : request.m_destinations) {
        m_destinations.push_back(destination->copy(destination, make_shared<monero_destination>()));
      }
    }
    m_payment_id = request.m_payment_id;
    m_priority = request.m_priority;
    m_ring_size = request.m_ring_size;
    m_fee = request.m_fee;
    m_account_index = request.m_account_index;
    m_subaddress_indices = request.m_subaddress_indices;
    m_unlock_time = request.m_unlock_time;
    m_can_split = request.m_can_split;
    m_do_not_relay = request.m_do_not_relay;
    m_note = request.m_note;
    m_recipient_name = request.m_recipient_name;
    m_below_amount = request.m_below_amount;
    m_sweep_each_subaddress = request.m_sweep_each_subaddress;
    m_key_image = request.m_key_image;
  }

  rapidjson::Value monero_send_request::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_priority != boost::none) monero_utils::addJsonMember("priority", m_priority.get(), allocator, root, value_num);
    if (m_ring_size != boost::none) monero_utils::addJsonMember("ringSize", m_ring_size.get(), allocator, root, value_num);
    if (m_account_index != boost::none) monero_utils::addJsonMember("accountIndex", m_account_index.get(), allocator, root, value_num);
    if (m_unlock_time != boost::none) monero_utils::addJsonMember("unlockTime", m_unlock_time.get(), allocator, root, value_num);
    if (m_below_amount != boost::none) monero_utils::addJsonMember("belowAmount", m_below_amount.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_payment_id != boost::none) monero_utils::addJsonMember("paymentId", m_payment_id.get(), allocator, root, value_str);
    if (m_note != boost::none) monero_utils::addJsonMember("note", m_note.get(), allocator, root, value_str);
    if (m_recipient_name != boost::none) monero_utils::addJsonMember("recipientName", m_recipient_name.get(), allocator, root, value_str);
    if (m_key_image != boost::none) monero_utils::addJsonMember("keyImage", m_key_image.get(), allocator, root, value_str);

    // set bool values
    if (m_can_split != boost::none) monero_utils::addJsonMember("canSplit", m_can_split.get(), allocator, root);
    if (m_do_not_relay != boost::none) monero_utils::addJsonMember("doNotRelay", m_do_not_relay.get(), allocator, root);
    if (m_sweep_each_subaddress != boost::none) monero_utils::addJsonMember("sweepEachSubaddress", m_sweep_each_subaddress.get(), allocator, root);

    // set sub-arrays
    if (!m_destinations.empty()) root.AddMember("destinations", monero_utils::to_rapidjson_val(allocator, m_destinations), allocator);
    if (!m_subaddress_indices.empty()) root.AddMember("subaddressIndices", monero_utils::to_rapidjson_val(allocator, m_subaddress_indices), allocator);

    // return root
    return root;
  }

  shared_ptr<monero_send_request> monero_send_request::deserialize(const string& send_request_json) {

    // deserialize send request json to property node
    std::istringstream iss = send_request_json.empty() ? std::istringstream() : std::istringstream(send_request_json);
    boost::property_tree::ptree node;
    boost::property_tree::read_json(iss, node);

    // convert request property tree to monero_send_request
    shared_ptr<monero_send_request> send_request = make_shared<monero_send_request>();
    for (boost::property_tree::ptree::const_iterator it = node.begin(); it != node.end(); ++it) {
      string key = it->first;
      if (key == string("destinations")) {
        boost::property_tree::ptree destinationsNode = it->second;
        for (boost::property_tree::ptree::const_iterator it2 = destinationsNode.begin(); it2 != destinationsNode.end(); ++it2) {
          shared_ptr<monero_destination> destination = make_shared<monero_destination>();
          monero_destination::from_property_tree(it2->second, destination);
          send_request->m_destinations.push_back(destination);
        }
      }
      else if (key == string("paymentId")) send_request->m_payment_id = it->second.data();
      else if (key == string("priority")) throw runtime_error("deserialize_send_request() priority not implemented");
      else if (key == string("ringSize")) send_request->m_ring_size = it->second.get_value<uint32_t>();
      else if (key == string("fee")) send_request->m_fee = it->second.get_value<uint64_t>();
      else if (key == string("accountIndex")) send_request->m_account_index = it->second.get_value<uint32_t>();
      else if (key == string("subaddressIndices")) for (boost::property_tree::ptree::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) send_request->m_subaddress_indices.push_back(it2->second.get_value<uint32_t>());
      else if (key == string("unlockTime")) send_request->m_unlock_time = it->second.get_value<uint64_t>();
      else if (key == string("canSplit")) send_request->m_can_split = it->second.get_value<bool>();
      else if (key == string("doNotRelay")) send_request->m_do_not_relay = it->second.get_value<bool>();
      else if (key == string("note")) send_request->m_note = it->second.data();
      else if (key == string("recipientName")) send_request->m_recipient_name = it->second.data();
      else if (key == string("belowAmount")) send_request->m_below_amount = it->second.get_value<uint64_t>();
      else if (key == string("sweepEachSubaddress")) send_request->m_sweep_each_subaddress = it->second.get_value<bool>();
      else if (key == string("keyImage")) send_request->m_key_image = it->second.data();
    }

    return send_request;
  }

  monero_send_request monero_send_request::copy() const {
    return monero_send_request(*this);
  }

  // ---------------------- MONERO INTEGRATED ADDRESS -------------------------

  rapidjson::Value monero_integrated_address::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    monero_utils::addJsonMember("standardAddress", m_standard_address, allocator, root, value_str);
    monero_utils::addJsonMember("paymentId", m_payment_id, allocator, root, value_str);
    monero_utils::addJsonMember("integratedAddress", m_integrated_address, allocator, root, value_str);

    // return root
    return root;
  }

  // -------------------- MONERO KEY IMAGE IMPORT RESULT ----------------------

  rapidjson::Value monero_key_image_import_result::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_height != boost::none) monero_utils::addJsonMember("height", m_height.get(), allocator, root, value_num);
    if (m_spent_amount != boost::none) monero_utils::addJsonMember("spentAmount", m_spent_amount.get(), allocator, root, value_num);
    if (m_unspent_amount != boost::none) monero_utils::addJsonMember("unspentAmount", m_unspent_amount.get(), allocator, root, value_num);

    // return root
    return root;
  }

  // ----------------------------- MONERO CHECK -------------------------------

  rapidjson::Value monero_check::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set bool values
    monero_utils::addJsonMember("isGood", m_is_good, allocator, root);

    // return root
    return root;
  }

  // --------------------------- MONERO CHECK TX ------------------------------

  rapidjson::Value monero_check_tx::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_check::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_num_confirmations != boost::none) monero_utils::addJsonMember("numConfirmations", m_num_confirmations.get(), allocator, root, value_num);
    if (m_received_amount != boost::none) monero_utils::addJsonMember("receivedAmount", m_received_amount.get(), allocator, root, value_num);

    // set bool values
    if (m_in_tx_pool != boost::none) monero_utils::addJsonMember("inTxPool", m_in_tx_pool.get(), allocator, root);

    // return root
    return root;
  }

  // ------------------------ MONERO CHECK RESERVE ----------------------------

  rapidjson::Value monero_check_reserve::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // serialize root from superclass
    rapidjson::Value root = monero_check::to_rapidjson_val(allocator);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_total_amount != boost::none) monero_utils::addJsonMember("totalAmount", m_total_amount.get(), allocator, root, value_num);
    if (m_unconfirmed_spent_amount != boost::none) monero_utils::addJsonMember("unconfirmedSpentAmount", m_unconfirmed_spent_amount.get(), allocator, root, value_num);

    // return root
    return root;
  }

  // --------------------------- MONERO MULTISIG ------------------------------

  rapidjson::Value monero_multisig_info::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    monero_utils::addJsonMember("threshold", m_threshold, allocator, root, value_num);
    monero_utils::addJsonMember("numParticipants", m_num_participants, allocator, root, value_num);

    // set bool values
    monero_utils::addJsonMember("isMultisig", m_is_multisig, allocator, root);
    monero_utils::addJsonMember("isReady", m_is_ready, allocator, root);

    // return root
    return root;
  }

  rapidjson::Value monero_multisig_init_result::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);
    if (m_multisig_hex != boost::none) monero_utils::addJsonMember("multisigHex", m_multisig_hex.get(), allocator, root, value_str);

    // return root
    return root;
  }

  rapidjson::Value monero_multisig_sign_result::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {
    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_signed_multisig_tx_hex != boost::none) monero_utils::addJsonMember("signedMultisigTxHex", m_signed_multisig_tx_hex.get(), allocator, root, value_str);

    // set sub-arrays
    if (!m_tx_hashes.empty()) root.AddMember("txHashes", monero_utils::to_rapidjson_val(allocator, m_tx_hashes), allocator);

    // return root
    return root;
  }

  // -------------------------- MONERO ADDRESS BOOK ---------------------------

  rapidjson::Value monero_address_book_entry::to_rapidjson_val(rapidjson::Document::AllocatorType& allocator) const {

    // create root
    rapidjson::Value root(rapidjson::kObjectType);

    // set num values
    rapidjson::Value value_num(rapidjson::kNumberType);
    if (m_index != boost::none) monero_utils::addJsonMember("index", m_index.get(), allocator, root, value_num);

    // set string values
    rapidjson::Value value_str(rapidjson::kStringType);
    if (m_address != boost::none) monero_utils::addJsonMember("address", m_address.get(), allocator, root, value_str);
    if (m_description != boost::none) monero_utils::addJsonMember("description", m_description.get(), allocator, root, value_str);
    if (m_payment_id != boost::none) monero_utils::addJsonMember("paymentId", m_payment_id.get(), allocator, root, value_str);

    // return root
    return root;
  }
}