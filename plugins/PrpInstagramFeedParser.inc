<?php

/**
 * @file
 * Contains PrpInstagramFeedParser and related classes.
 */

class PrpInstagramFeedParser extends FeedsParser {

  /**
   * Implementation of FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    $mappings = $source->importer->processor->config['mappings'];
    $raw = trim($fetcher_result->getRaw());

    $result = new FeedsParserResult();

    // Set link so we can set the result link attribute.
    $fetcher_config = $source->getConfigFor($source->importer->fetcher);
    $result->link = $fetcher_config['prp_instagram_feed_oauth_client_id'];

    $array = json_decode($raw, TRUE);

    // Get the fetcher class.
    $classType = get_class($fetcher_result);

    // Get the min_tag_id and store it in prp_instagram_next_min_id to be used
    // in the fetcher.
    if ($classType == 'PrpInstagramFeedHashtagFetcherResult') {
      $min_id = $array['pagination']['min_tag_id'];
      $hash = $fetcher_result->getHashtag();
      $getHashTags = variable_get('prp_instagram_next_min_id', NULL);

      if ($min_id == NULL) {
        if ($getHashTags != NULL) {
          variable_set('prp_instagram_next_min_id', $getHashTags);
        }
        else {
          $getHashTags[$hash] = '';
          variable_set('prp_instagram_next_min_id', $getHashTags);
        }
      }
      else {
        $getHashTags[$hash] = $min_id;
        variable_set('prp_instagram_next_min_id', $getHashTags);
      }
    }

    // Support JSON lines format.
    if (!is_array($array)) {
      $raw = preg_replace('/(?:\\\\u[0-9a-fA-Z]{4})+/', '', $raw);
      $raw = '[' . $raw . ']';
      $array = json_decode($raw, TRUE);
    }

    $blocked_instagram_profiles = variable_get('blocked_instagram_profiles','');
    $blocked_instagram_profiles_array = array();
    if (!empty($blocked_instagram_profiles)) {
      $blocked_instagram_profiles_array = explode( ",", $blocked_instagram_profiles);
    }


    if (is_array($array) && isset($array['data'])) {
      $all_items = $array['data'];
      foreach ($all_items as $item) {
        $parsed_item = $variables = array();
        foreach ($mappings as $map) {
          $source = $map['source'];
          $parsed = $this->parseSourceElement($item, $map);
          if (!empty($parsed)) {
            // Variable sunstitution can't handle arrays.
            if (!is_array($parsed)) {
              $variables['{' . $map['target'] . '}'] = $parsed;
            }
            else {
              $variables['{' . $map['target'] . '}'] = '';
            }
            $parsed_item[$source] = $parsed;
          }
        }
        if (!in_array($parsed_item['user:username'], $blocked_instagram_profiles_array)){
           $result->items[] = $parsed_item;
        }
      }
    }
    else {
      throw new Exception(t('There was an error decoding the JSON document.'));
    }

    // Check and remove any duplicates from the results
    if (is_array($result->items)){
      $result->items = array_map("unserialize", array_unique(array_map("serialize", $result->items)));
    }

    return $result;
  }

  protected function parseSourceElement($item, $map) {
    $source = $map['source'];
    if ($source == 'type_blog') {
      $parsed = 'Instagram';
    }
    else {
      $source_array = explode(':', $source);
      $parsed = $item;
      $source_array = explode(':', $source);
      foreach ($source_array as $element) {
        $parsed = $parsed[$element];
      }
    }

    return ($source != 'created_time') ? utf8_encode($parsed) : $parsed;
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = $this->sourceForm($this->config);

    return $form;
  }

  /**
   * Define defaults.
   */
  public function configDefaults() {
    return array(
      'sources' => array(),
    );
  }

  /**
   * Source form.
   */
  public function getMappingSources() {
    $sources = array();
    $content_type = feeds_importer($this->id)->config['content_type'];
    return $this->loadInstagramSource();
  }

  protected function loadInstagramSource() {
    return array(
      'id' => array(
        'name' => t('Instragram ID'),
      ),
      'created_time' => array(
        'name' => t('Instagram: created_time'),
      ),
      'user:username' => array(
        'name' => t('Instagram user: username'),
      ),
      'user:profile_picture' => array(
        'name' => t('Instagram user: profile_picture'),
        'desciption' => t('Profile picture of instagram user'),
      ),
      'user:full_name' => array(
        'name' => t('Instagram user: full_name')
      ),
      'link' => array(
        'name' => t('Instagram: link'),
        'description' => t('Link to instragram photo'),
      ),
      'images:standard_resolution:url' => array(
        'name' => t('Link to  standart resolution image'),
      ),
      'type_blog' => array(
        'name' => t('Return Instagram'),
      ),
      'comments:count' => array(
        'name' => t('Comments count'),
      ),
      'likes:count' => array(
        'name' => t('Likes coount'),
      ),
    );
  }

}
