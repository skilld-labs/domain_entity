<?php

/**
 * @file
 * Hooks provided by the domain_entity module.
 */


/**
 * Implements hook_domain_entity_allowed_entity_types_alter().
 *
 * Allows modules to alter domain entity enabled entity,
 * and their widget type by bundle.
 *
 * @param array $allowed_entity_types
 *   The domain entity settings array.
 */
function hook_domain_entity_allowed_entity_types_alter(&$allowed_entity_types) {
  if (isset($allowed_entity_types['commerce_order'])
      && strpos(current_path(), "all/my/commerce_order") === 0) {
    // Disable domain access rules on commerce_order on the path all/my/commerce_order.
    unset($allowed_entity_types['commerce_order']);
  }
}
