<?php

/**
 * @file
 * Hooks provided by the domain_entity module.
 */


/**
 * Alter domain enabled entity types.
 *
 * Allows modules to alter domain entity enabled entity,
 * and their default assignation values by bundle.
 *
 * This array is structured as follow:
 * array(
 *   entity_type => array(
 *     bundle => array(
 *       widget_behaviour => array(
 *         default_value => default_value,
 *       ),
 *     ),
 *     other bundle => etc..
 *   ),
 *   other entity => etc..
 * );
 *
 * Example :
 * array(
 *   'commerce_order' => array(
 *     'commerce_order' => array(
 *       DOMAIN_ENTITY_BEHAVIOR_USER => array(
 *         DOMAIN_ACTIVE => DOMAIN_ACTIVE
 *       ),
 *     ),
 *   ),
 * );
 *
 * You can't change the widget by this hook, there where no effects.
 *
 * You can :
 * You can unset an entity type from the array to disable query altering
 * on this particular entity types.
 *
 * You can change the default assignation values of each entity types bundles.
 *
 * you can export configuration at install with that hook but you need to submit
 * the domain configuration form after the installation. Except if you export
 * the domain fields with feature or create it yourself after the install,
 * see domain_entity_types_enable_domain_field($entity_types).
 *
 * @param array $allowed_entity_types
 *   (alterable) The domain entity settings array.
 */
function hook_domain_entity_allowed_entity_types_alter(&$allowed_entity_types) {
  if (isset($allowed_entity_types['commerce_order'])
      && strpos(current_path(), "all/my/commerce_order") === 0) {
    // Disable domain access rules on commerce_order on
    // the path all/my/commerce_order.
    unset($allowed_entity_types['commerce_order']);
  }
}

/**
 * Called when the field widget is submit for saving and
 * domain entity form validate function manipulate the items before saving.
 *
 * @param array $items
 *   (alterable) The domain field value that is gonna be saved.
 * @param $form_state
 *   The form state of the form that hold the field domain_entity.
 * @param $form
 *   The form that hold the field domain_entity.
 */
function hook_domain_entity_widget_multiple_values_form_validate_alter(&$items, $form_state, $form) {
  $domain_entity_field_name = $form_state['domain_entity_field_name'];
  $form_field_values = isset($form_state['values'][$form_state['domain_entity_field_name']]) ? $form_state['values'][$form_state['domain_entity_field_name']] : NULL;
}
