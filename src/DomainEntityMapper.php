<?php

namespace Drupal\domain_entity;

use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Entity\FieldableEntityInterface;
use Drupal\Core\Field\FieldStorageDefinitionInterface;
use Drupal\field\Entity\FieldConfig;

/**
 * Provides fields operations for domain entity module fields.
 */
class DomainEntityMapper {

  /**
   * The name of the access control field.
   */
  const FIELD_NAME = 'domain_access';

  /**
   * Domain entity behavior widget type, add a hidden field on entity.
   *
   * Entity is automatically assigned to the current domain (hidden for user).
   */
  const BEHAVIOR_AUTO = 'auto';

  /**
   * Domain entity behavior widget type, add a field on entity creation form.
   *
   * Allowing user to choose entity affiliation on creation/update form.
   */
  const BEHAVIOR_USER = 'user';

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManagerInterface
   */
  protected $entityTypeManager;

  /**
   * Creates a new DomainEntityMapper object.
   *
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The entity type manager.
   */
  public function __construct(EntityTypeManagerInterface $entity_type_manager) {
    $this->entityTypeManager = $entity_type_manager;
  }

  /**
   * Returns entity types that have domain access field storage.
   *
   * @return \Drupal\Core\Entity\EntityTypeInterface[]
   *   Keyed array of enabled entity types.
   */
  public function getEnabledEntityTypes() {
    $result = [];
    $types = $this->getEntityTypes();
    foreach ($types as $id => $type) {
      if ($this->loadFieldStorage($id)) {
        $result[$id] = $type;
      }
    }
    return $result;
  }

  /**
   * Returns fieldable entity type definitions.
   *
   * @return \Drupal\Core\Entity\EntityTypeInterface[]
   *   The fieldable entity types.
   */
  public function getEntityTypes() {
    $entity_types = $this->entityTypeManager->getDefinitions();
    $result = [];
    foreach ($entity_types as $entity_type_id => $entity_type) {
      // @todo Fix https://www.drupal.org/node/2842808 for 8.3.x core.
      if ($entity_type->isSubclassOf(FieldableEntityInterface::class)) {
        $result[$entity_type_id] = $entity_type;
      }
    }
    return $result;
  }

  /**
   * Loads field storage config.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   *
   * @return \Drupal\field\Entity\FieldStorageConfig|null
   *   The field storage or NULL.
   */
  public function loadFieldStorage($entity_type_id) {
    $storage = $this->entityTypeManager->getStorage('field_storage_config');
    return $storage->load($entity_type_id . '.' . self::FIELD_NAME);
  }

  /**
   * Loads field from entity bundle.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   * @param string $bundle
   *   The entity type bundle name.
   *
   * @return \Drupal\field\Entity\FieldConfig|null
   *   The field or NULL.
   */
  public function loadField($entity_type_id, $bundle) {
    $storage = $this->entityTypeManager->getStorage('field_config');
    return $storage->load($entity_type_id . '.' . $bundle . '.' . self::FIELD_NAME);
  }

  /**
   * Deletes field storage.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   */
  public function deleteFieldStorage($entity_type_id) {
    $field_storage = $this->loadFieldStorage($entity_type_id);
    if ($field_storage) {
      $field_storage->delete();
    }
  }

  /**
   * Creates domain fields.
   *
   * @param string $entity_type
   *   The entity type machine name.
   * @param string $bundle
   *   The entity type's bundle.
   */
  public function addDomainField($entity_type, $bundle) {
    $field_storage = $this->createFieldStorage($entity_type);
    $field = FieldConfig::loadByName($entity_type, $bundle, self::FIELD_NAME);
    if (empty($field)) {
      $field = [
        'label' => 'Domain Access',
        // @Todo Add better naming for entities without bundles.
        'description' => 'Select the affiliate domain(s). If nothing was selected: Affiliated to all domains.',
        'bundle' => $bundle,
        'required' => FALSE,
        'field_storage' => $field_storage,
        'default_value_callback' => 'domain_entity_field_default_domains',
      ];

      $field = $this->entityTypeManager->getStorage('field_config')
        ->create($field);
      $field->save();

      // Assign widget settings for the 'default' form mode.
      $entity_form_display = $this->entityTypeManager->getStorage('entity_form_display')->load($entity_type . '.' . $bundle . '.default');
      if ($entity_form_display) {
        /** @var \Drupal\Core\Entity\Display\EntityFormDisplayInterface $entity_form_display */
        $entity_form_display->setComponent(self::FIELD_NAME, [
          'type' => 'options_buttons',
        ])->save();
      }

      // Assign display settings for the 'default' view mode.
      $entity_view_display = $this->entityTypeManager->getStorage('entity_view_display')->load($entity_type . '.' . $bundle . '.default');
      if ($entity_view_display) {
        /** @var \Drupal\Core\Entity\Display\EntityViewDisplayInterface $entity_view_display */
        $entity_view_display->removeComponent(self::FIELD_NAME)->save();
      }
    }
  }

  /**
   * Creates field storage.
   *
   * @param string $entity_type_id
   *   The entity type ID.
   *
   * @return \Drupal\field\Entity\FieldStorageConfig
   *   The field storage.
   */
  public function createFieldStorage($entity_type_id) {
    if ($field_storage = $this->loadFieldStorage($entity_type_id)) {
      // Prevent creation of existing field storage.
      return $field_storage;
    }
    $storage = $this->entityTypeManager->getStorage('field_storage_config');
    $field_storage = $storage->create([
      'entity_type' => $entity_type_id,
      'field_name' => self::FIELD_NAME,
      'type' => 'entity_reference',
      // @todo Polish to enable UI and optimize storage.
      'persist_with_no_fields' => TRUE,
      'locked' => FALSE,
    ]);
    $field_storage
      ->setCardinality(FieldStorageDefinitionInterface::CARDINALITY_UNLIMITED)
      ->setSetting('target_type', 'domain')
      ->save();
    return $field_storage;
  }

}
