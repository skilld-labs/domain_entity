Overview

This module provide a solution to add Domain Access on entity.

You can use this module to restrict access for entity as you do with node.

You can choose for each bundle the field behavior you want to use :

    Affiliate automatically created entity to current domain
    User choose affiliate (default value is current domain)

The access rule is basic:

    - entity data is only accessible on his domain(s).
    This is the default behavior for administration and front office.

    - User with specific permission can access and/or edit content of 
    multiple domain (with domain Access editor user assigned domain)

    - This module provide a block to filter entities by domain in 
    administration pages, only accessible by user editor of multiple domains 
    with the specific (same as above) permission.
    For example it can be used to filter taxonomy terms on list pages, 
    the filter is generic and can be used with entity that are enabled 
    in Domain Access entities settings.

Features

- Enable Domain Access on entities
- Filter entity by domain in every administration pages. 
  (it's possible to filtering views, taxonomy_term list)
- UI for enabling domain access on entity type
Requirements

- Domain Access http://drupal.org/project/domain
- Entity API http://drupal.org/project/entity
Setup

- Enable domain_entity
- setup at least one domain..
- Access to admin/structure/domain/entities
- Use the forms to enable domain on entities, you can choose 
  the behavior widget used for each bundle (for the moment existing 
  content of your website will be assigned to the default value)
- You can assign permission to roles that you allow to edit 
  multiple domains entities. Allowing the role to see and filter 
  each administration pages with domain affiliated entities
