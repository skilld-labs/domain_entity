<?php

/**
 * Implements hook_rules_condition_info().
 */
function domain_entity_rules_condition_info() {

  $conditions = array();

  $conditions['domain_entity_rules_entity_is_affiliate_to_domain'] = array(
    'label' => t("Entity is affiliate to domain (machine name)"),
    'parameter' => array(
      'order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
      ),
      'machine_name' => array(
        'type' => 'text',
        'label' => t('Domain Access: Machine name'),
      ),
    ),
    'group' => t('Domain entity'),
    'callbacks' => array(
      'execute' => 'domain_entity_rules_entity_is_affiliate_to_domain',
    ),
  );

  return $conditions;
}

/**
 * Rules condition: checks to see if the given order's domain id,
 * is equals to a given domain machine_name
 */
function domain_entity_rules_entity_is_affiliate_to_domain($order, $machine_name) {
  $domain_id = domain_load_domain_id($machine_name);

  $field_instance = domain_entity_entity_field_instance('commerce_order');
  $field_instance_name = $field_instance['name'];
  $values = field_get_items('commerce_order', $order, $field_instance_name);

  // return test
  foreach ($values as $value) {
    if ($value['domain_id'] == $domain_id) {
      return TRUE;
    }
  }

  return FALSE;
}