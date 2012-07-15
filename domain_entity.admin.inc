<?php

/**
 * Domain Access Entity settings form
 * @param $form
 * @param $form_state
 */
function domain_entity_ui($form, &$form_state) {
  // get the domain_entity_allowed_entity_types values
  $default_values = domain_entity_allowed_entity_types();
  $entities_info = entity_get_info();
  // Domain access take care of it:
  unset($entities_info['node']);
  // I think this necessit an other module (login, register handling.. to be tested):
  unset($entities_info['user']);
  // don't know too much about file entity.., for the moment disable it:
  unset($entities_info['file']);
  // don't know too much about taxonomy vocabulary entity.., for the moment disable it:
  unset($entities_info['taxonomy_vocabulary']);
  // Set the form.
  $form = array();
  $form['header'] = array(
    '#markup' => t('Choose wich entities are under Domain Access control, and choose domain entity widget behavior of bundles'),
  );
  $domain_entity_options = array(
    DOMAIN_ENTITY_BEHAVIOR_AUTO => t('Affiliate automatically created entity to current domain'),
    DOMAIN_ENTITY_BEHAVIOR_USER => t('User choose affiliate (default value is current domain)'),
  );
  $form['entities'] = array(
    '#prefix' => '<div id="domain-entity-ui-form-ajax">',
    '#suffix' => '</div>',
  );
  // Now build the form elements.
  foreach ($entities_info as $entity_name => $entity_info) {
    $form['entities'][$entity_name] = array(
      '#title' => '&#x2713; ' . $entity_info['label'] . ' (' . $entity_name . ')',
      '#type' => 'fieldset',
    );
    $form['entities'][$entity_name][$entity_name . '_value'] = array(
      '#title' => t('Activate domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'],'@entity_name' => $entity_name)),
      '#type' => 'checkbox',
      '#default_value' => isset($default_values[$entity_name]),
      '#ajax' => array(
        'event' => 'change',
        'callback' => 'domain_entity_ui_form_ajax_callback',
        'wrapper' => 'domain-entity-ui-form-ajax',
        'effect' => 'none',
      ),
    );
    foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
      $form['entities'][$entity_name]['bundles'][$entity_name . '_' . $bundle_name . '_behavior'] = array(
        '#title' => t('Choose wich behavior Domain entity must use with bundle @bundle_label (\'@bundle_name\')', array('@bundle_label' => $bundle_info['label'], '@bundle_name' => $bundle_name)),
        '#type' => 'select',
        '#options' => $domain_entity_options,
        '#default_value' => (isset($default_values[$entity_name]) && isset($default_values[$entity_name][$bundle_name])) ? array($default_values[$entity_name][$bundle_name] => $default_values[$entity_name][$bundle_name]) : array(DOMAIN_ENTITY_BEHAVIOR_USER => DOMAIN_ENTITY_BEHAVIOR_USER)
      );
    }
    if (isset($form_state['triggering_element'])) {
      if (empty($form_state['values']) || !isset($form_state['values'][$entity_name. '_value']) || !$form_state['values'][$entity_name. '_value']) {
        $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
        $form['entities'][$entity_name]['#type'] = 'markup';
        $form['entities'][$entity_name]['#weight'] = 1000;
      } else {
        $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
        $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'],'@entity_name' => $entity_name));
        $form['entities'][$entity_name]['#type'] = 'fieldset';
        // Check if content of this type exist in DB, if so prompt a warning
        if (isset($entity_info['base table'])) {
          $entity_key = $entity_info['entity keys']['id'];
          $query = db_select($entity_info['base table'], 'b');
          $query->addField('b', $entity_key);
          if ($query->execute()->fetchField()) {
            $form['entities'][$entity_name][$entity_name . '_value']['#description'] =
              '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
              t('* Warning you have content of this type in your database, all unassigned content will be assigned to the defaut domain') .
              '</div>';
          }
        }
      }
      $form['entities']['#prefix'] =
        '<div id="domain-entity-ui-form-ajax">' .
        '<div id="edit-changed" class="view-changed messages warning form-wrapper">' .
        t('* Changes are not saved before you submit this form') .
        '</div>';
    } elseif(!isset($default_values[$entity_name])) {
      $form['entities'][$entity_name]['bundles']['#access'] = FALSE;
      $form['entities'][$entity_name]['#type'] = 'markup';
      $form['entities'][$entity_name]['#weight'] = 1000;
    } else {
      $form['entities'][$entity_name]['bundles']['#access'] = TRUE;
      $form['entities'][$entity_name][$entity_name . '_value']['#title'] = t('un-check for disabling domain access on @entity_label (@entity_name)', array('@entity_label' => $entity_info['label'],'@entity_name' => $entity_name));
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
 * Ajax form callback,
 */
function domain_entity_ui_form_ajax_callback($form, $form_state) {
  return $form['entities'];

}

/**
 * Validate handler for the domain overview form.
 *
function domain_entity_ui_validate($form, &$form_state) {

}//*/

/**
 * Submit handler for the domain entity form.
 */
function domain_entity_ui_submit($form, &$form_state) {
  $values = $form_state['values'];
  // Set the form.
  $entities_info = entity_get_info();
  // Domain access take care of it:
  unset($entities_info['node']);
  // I think this necessit an other module (login, register handling.. to be tested):
  unset($entities_info['user']);
  // don't know too much about taxonomy vocabulary entity.., for the moment disable it:
  unset($entities_info['file']);
  unset($entities_info['taxonomy_vocabulary']);
  $domain_entity_allowed_entity_types = array();
  foreach ($entities_info as $entity_name => $entity_info) {
    if($values[$entity_name . '_value']) {
      $domain_entity_allowed_entity_types [$entity_name] = array();
      foreach($entity_info['bundles'] as $bundle_name => $bundle_info) {
        $domain_entity_allowed_entity_types[$entity_name][$bundle_name] = $values[$entity_name . '_' . $bundle_name . '_behavior'];
      }
    }
  }
  variable_set('domain_entity_allowed_entity_types', $domain_entity_allowed_entity_types);
  drupal_set_message(t('Domain settings updated'));
  domain_entity_types_enable_domain_field($domain_entity_allowed_entity_types);
}

/**
 * Return the filter administration pages entities widget
 */
function domain_entity_domain_switcher_get_content() {
  $links = array();
  $domains = domain_domains();
  global $user;
  $user_granted_domain_ids = domain_get_user_domains($user);
  foreach ($domains as $domain) {
    if (in_array($domain['domain_id'], $user_granted_domain_ids)) {
      if(isset($_GET['domain']) && $domain['domain_id'] == $_GET['domain']) {
        $links[] = '<strong>' . t('This page are listing only entites affiliated from the site: @sitename', array('@sitename' => $domain['sitename'])) . '</strong>';
      } else {
        $links[] = l(t('See only entity affiliated from the site @sitename on this page', array('@sitename' => $domain['sitename'])), current_path(), array('query' => array('domain' => $domain['domain_id'])));
      }
    }
  }
  return theme_item_list(array(
    'items' =>$links,
    'attributes' => array(),
    'title' => '',
    'type' => 'ul'
  ));
}
