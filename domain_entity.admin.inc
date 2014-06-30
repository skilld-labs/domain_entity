<?php

/**
 * @file
 * Domain Access Entity admininistration settings / form
 */

/**
 * Domain Access Entity settings form.
 *
 * @param array $form
 *   Form array.
 * @param array $form_state
 *   Form state array.
 */
function domain_entity_ui($form, &$form_state) {
  // Get the domain_entity_allowed_entity_types values:
  $default_values = domain_entity_allowed_entity_types();

  // Do not allow domain access on entity that are actually bundle.
  $entities_info = entity_get_info();
  foreach ($entities_info as $entity_type => $entity_info) {
    if (isset($entity_info['bundle of']) || !isset($entity_info['entity keys']['bundle']) || empty($entity_info['entity keys']['bundle'])) {
      unset($entities_info[$entity_type]);
    }
  }
  // Domain access take care of it:
  unset($entities_info['node']);

  // Set the form:
  $form = array();
  $form['header'] = array(
    '#markup' => t('Choose which entities are under Domain Access control, and choose domain entity widget behavior of bundles'),
  );
  $form['domain_entity_bypass_access_conditions'] = array(
    '#title' => t('Disable access rules from this module. (You can use this settings to disable the query alter, for troubleshooting)'),
    '#type' => 'checkbox',
    '#description' => t('When this checkbox is checked, your entities must be accessible on all domains'),
    '#default_value' => variable_get('domain_entity_bypass_access_conditions', FALSE),
    '#weight' => 3,
  );
  $domain_entity_options = array(
    DOMAIN_ENTITY_BEHAVIOR_AUTO => t('Affiliate automatically created entity to a value (no widget on entity creation form, auto-assignation)'),
    DOMAIN_ENTITY_BEHAVIOR_USER => t('User choose affiliate, with a default value (form widget on the entity creation form)'),
  );
  $form['entities'] = array(
    '#prefix' => '<div id="domain-entity-ui-form-ajax">',
    '#suffix' => '</div>',
  );
  // Construct widget default values options:
  $site_domains = domain_domains();
  $checkboxes = array(DOMAIN_ACTIVE => t('Current domain'), DOMAIN_ALL => t('Send to all affiliates'));
  foreach ($site_domains as $site_domain) {
    $checkboxes[$site_domain['domain_id']] = $site_domain['sitename'];
  }

  // Now build the form elements.
  foreach ($entities_info as $entity_name => $entity_info) {
    $title_prefix = isset($default_values[$entity_name]) ? '&#x2713; ' : ' <strong style="color:red;">*</strong> ';
    $form['entities'][$entity_name] = array(
      '#title' => $title_prefix . $entity_info['label'] . ' (' . $entity_name . ')',
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => isset($default_values[$entity_name]),
    );
    $form['entities'][$entity_name][$entity_name . '_value'] = array(
      '#title' => t('Activate domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name)),
      '#type' => 'checkbox',
      '#default_value' => isset($default_values[$entity_name]),
      '#ajax' => array(
        'event' => 'change',
        'callback' => 'domain_entity_ui_form_ajax_callback',
        'wrapper' => 'domain-entity-ui-form-ajax',
        'effect' => 'none',
      ),
    );
    if (!$entity_info['fieldable']) {
      $form['entities'][$entity_name][$entity_name . '_value']['#title'] .= '<br/><small>' . t('By default, this entity is not fieldable, but he looks like we can change that and make this entity fieldable.') . '</small>';
    }
    foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
      $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_behavior'] = array(
        '#prefix' => '<fieldset class="collapsible form-wrapper collapsed"><legend><span class="fieldset-legend">' . $bundle_info['label'] . '</span></legend><div class="fieldset-wrapper">',
        '#title' => t("Choose which behavior must be used with the bundle @bundle_label ('@bundle_name')", array('@bundle_label' => $bundle_info['label'], '@bundle_name' => $bundle_name)),
        '#type' => 'select',
        '#options' => $domain_entity_options,
        '#default_value' => (isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) ? array(key($default_values[$entity_name][$bundle_name]) => key($default_values[$entity_name][$bundle_name])) : array(DOMAIN_ENTITY_BEHAVIOR_USER => DOMAIN_ENTITY_BEHAVIOR_USER),
      );
      // Add a checkboxes for domain id's:
      $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'] = array(
        '#type' => 'checkboxes',
        '#options' => $checkboxes,
        '#title' => t("default domain value(s) for the bundle: @bundle_label ('@bundle_name')", array('@bundle_label' => $bundle_info['label'], '@bundle_name' => $bundle_name)),
        '#default_value' => (isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) ? reset($default_values[$entity_name][$bundle_name]) : array(DOMAIN_ACTIVE => DOMAIN_ACTIVE),
        '#required' => TRUE,
        '#suffix' => '</div></fieldset>',
        '#ajax' => array(
          'event' => 'change',
          'callback' => 'domain_entity_ui_form_ajax_callback',
          'wrapper' => 'domain-entity-ui-form-ajax',
        ),
      );

      if (!isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values']) && isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) {
        $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'] = reset($default_values[$entity_name][$bundle_name]);
      }

      if (!empty($form_state['values'])) {
        if (isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'])) {
          if ((isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ACTIVE]) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ACTIVE] === DOMAIN_ACTIVE) || (isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ALL]) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ALL] === DOMAIN_ALL)) {
            foreach ($site_domains as $site_domain) {
              $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][$site_domain['domain_id']] = 0;
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'][$site_domain['domain_id']]['#disabled'] = TRUE;
            }
            if ((isset($form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ACTIVE]) && $form_state['values'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ACTIVE] === DOMAIN_ACTIVE)) {
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ALL]['#disabled'] = TRUE;
            }
            else {
              $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_default_values'][DOMAIN_ACTIVE]['#disabled'] = TRUE;
            }
          }
        }
      }
    }
    if (isset($form_state['triggering_element'])) {
      if (empty($form_state['values']) || !isset($form_state['values'][$entity_name . '_value']) || !$form_state['values'][$entity_name . '_value']) {
        $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
        $form['entities'][$entity_name]['#type'] = 'markup';
        $form['entities'][$entity_name]['#weight'] = 1000;
      }
      else {
        $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
        $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name), all entities domain assignation values will be deleted.', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name));
        $form['entities'][$entity_name]['#type'] = 'fieldset';
        // Check if content of this type exist in DB, if so prompt a warning:
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', $entity_name);
        if ($query->execute()) {
          $form['entities'][$entity_name][$entity_name . '_value']['#description']
           = '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
           t('* Beware you have entities of this type in your database, all unassigned entities will be assigned to the choosen default domain value(s), if you select "current domain" the unassigned entities will be assigned to the current domain. You can change the default value afterway without altering the existing entities domain value(s).') .
           '</div>';
        }
      }
      $form['entities']['#prefix']
        = '<div id="domain-entity-ui-form-ajax">' .
        '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
        t('* Changes are not saved before you submit this form') .
        '</div>';
    }
    elseif (!isset($default_values[$entity_name])) {
      $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
      $form['entities'][$entity_name]['#type'] = 'markup';
      $form['entities'][$entity_name]['#weight'] = 1000;
    }
    else {
      $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
      $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name), all entities domain assignation values will be deleted.', array('@entity_label' => $entity_info['label'], '@entity_name' => $entity_name));
      $form['entities'][$entity_name]['#type'] = 'fieldset';
    }

  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Ajax form callback.
 *
 * @param array $form
 *   Form array.
 *
 * @param array $form_state
 *   Form state array.
 *
 * @return array
 *   Form array of the elements to be updated.
 */
function domain_entity_ui_form_ajax_callback($form, &$form_state) {
  return $form['entities'];

}

/**
 * Submit handler for the domain entity form.
 *
 * @param array $form
 *   Form array.
 *
 * @param array $form_state
 *   Form state array.
 */
function domain_entity_ui_submit($form, &$form_state) {
  $values = $form_state['values'];
  $old_domain_entity_allowed_entity_types = variable_get('domain_entity_allowed_entity_types', array());

  // Save the disable access conditions value.
  variable_set('domain_entity_bypass_access_conditions', $values['domain_entity_bypass_access_conditions']);

  // Do not allow domain access on entity that are actually bundle.
  $entities_info = entity_get_info();
  foreach ($entities_info as $entity_type => $entity_info) {
    if (isset($entity_info['bundle of']) || !isset($entity_info['entity keys']['bundle']) || empty($entity_info['entity keys']['bundle'])) {
      unset($entities_info[$entity_type]);
    }
  }
  // Domain access take care of it:
  unset($entities_info['node']);

  // Just a bundle entity:
  unset($entities_info['taxonomy_vocabulary']);

  // Set the form.
  $domain_entity_allowed_entity_types = array();
  foreach ($entities_info as $entity_name => $entity_info) {
    if ($values[$entity_name . '_value']) {
      $domain_entity_allowed_entity_types[$entity_name] = array();
      foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
        $default_values = array();
        foreach ($values[$entity_name . '_' . $bundle_name . '_default_values'] as $key => $value) {
          if ($key == $value && $key !== DOMAIN_ACTIVE && $key !== DOMAIN_ALL) {
            $default_values[$key] = $value;
          }
          if ($key === DOMAIN_ACTIVE && $value === DOMAIN_ACTIVE) {
            $default_values = array($key => $value);
            break;
          }
          if ($key === DOMAIN_ALL && $value === DOMAIN_ALL) {
            $default_values = array($key => $value);
            break;
          }
        }
        // Change the widget of the fields if needed.
        $domain_entity_allowed_entity_types[$entity_name][$bundle_name] = array($values[$entity_name . '_' . $bundle_name . '_behavior'] => $default_values);
        if (in_array($entity_name, array_keys($old_domain_entity_allowed_entity_types))) {
          foreach ($domain_entity_allowed_entity_types[$entity_name] as $bundle_name => $bundle_info) {
           if (isset($old_domain_entity_allowed_entity_types[$entity_name][$bundle_name]) &&
          	    key($bundle_info) != key($old_domain_entity_allowed_entity_types[$entity_name][$bundle_name])) {
              $field_name = domain_entity_get_entity_field_name($entity_name);
              $instance = field_read_instance($entity_name, $field_name, $bundle_name);
              $instance['widget']['type'] = key($bundle_info);
              field_update_instance($instance);
          	}
          }
        }
      }
    }
  }
  // Uninstall disabled entity type (remove the field and instances).
  if (!empty($old_domain_entity_allowed_entity_types)) {
    $uninstall_entity_types = array_diff(array_keys($old_domain_entity_allowed_entity_types), array_keys($domain_entity_allowed_entity_types));
    if (!empty($uninstall_entity_types)) {
      foreach ($uninstall_entity_types as $uninstall_entity_type) {
        $field_name = domain_entity_get_entity_field_name($uninstall_entity_type);
        foreach ($old_domain_entity_allowed_entity_types[$uninstall_entity_type] as $bundle_name => $data) {
          $instance = field_info_instance($uninstall_entity_type, $field_name, $bundle_name);
          field_delete_instance($instance, TRUE);
          drupal_set_message(t('Domain access has been disabled on %entity_name', array('%entity_name' => $uninstall_entity_type)));
        }
      }
      field_delete_field($field_name);
    }
  }
  variable_set('domain_entity_allowed_entity_types', $domain_entity_allowed_entity_types);
  domain_entity_types_enable_domain_field($domain_entity_allowed_entity_types);
  drupal_set_message(t('Domain entity settings updated'));
}
